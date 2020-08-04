// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ALLOC_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ALLOC_H_

// DESCRIPTION
// PartitionRoot::Alloc() and PartitionRoot::Free() are approximately analagous
// to malloc() and free().
//
// The main difference is that a PartitionRoot object must be supplied to these
// functions, representing a specific "heap partition" that will be used to
// satisfy the allocation. Different partitions are guaranteed to exist in
// separate address spaces, including being separate from the main system
// heap. If the contained objects are all freed, physical memory is returned to
// the system but the address space remains reserved.  See PartitionAlloc.md for
// other security properties PartitionAlloc provides.
//
// THE ONLY LEGITIMATE WAY TO OBTAIN A PartitionRoot IS THROUGH THE
// PartitionAllocator classes. To minimize the instruction count to the fullest
// extent possible, the PartitionRoot is really just a header adjacent to other
// data areas provided by the allocator class.
//
// The constraints for PartitionRoot::Alloc() are:
// - Multi-threaded use against a single partition is ok; locking is handled.
// - Allocations of any arbitrary size can be handled (subject to a limit of
//   INT_MAX bytes for security reasons).
// - Bucketing is by approximate size, for example an allocation of 4000 bytes
//   might be placed into a 4096-byte bucket. Bucket sizes are chosen to try and
//   keep worst-case waste to ~10%.
//
// The allocators are designed to be extremely fast, thanks to the following
// properties and design:
// - Just two single (reasonably predicatable) branches in the hot / fast path
//   for both allocating and (significantly) freeing.
// - A minimal number of operations in the hot / fast path, with the slow paths
//   in separate functions, leading to the possibility of inlining.
// - Each partition page (which is usually multiple physical pages) has a
//   metadata structure which allows fast mapping of free() address to an
//   underlying bucket.
// - Supports a lock-free API for fast performance in single-threaded cases.
// - The freelist for a given bucket is split across a number of partition
//   pages, enabling various simple tricks to try and minimize fragmentation.
// - Fine-grained bucket sizes leading to less waste and better packing.
//
// The following security properties could be investigated in the future:
// - Per-object bucketing (instead of per-size) is mostly available at the API,
// but not used yet.
// - No randomness of freelist entries or bucket position.
// - Better checking for wild pointers in free().
// - Better freelist masking function to guarantee fault on 32-bit.

#include <limits.h>
#include <string.h>

#include <atomic>

#include "base/allocator/partition_allocator/checked_ptr_support.h"
#include "base/allocator/partition_allocator/page_allocator.h"
#include "base/allocator/partition_allocator/partition_alloc_check.h"
#include "base/allocator/partition_allocator/partition_alloc_constants.h"
#include "base/allocator/partition_allocator/partition_alloc_forward.h"
#include "base/allocator/partition_allocator/partition_bucket.h"
#include "base/allocator/partition_allocator/partition_cookie.h"
#include "base/allocator/partition_allocator/partition_direct_map_extent.h"
#include "base/allocator/partition_allocator/partition_page.h"
#include "base/allocator/partition_allocator/partition_tag.h"
#include "base/allocator/partition_allocator/spin_lock.h"
#include "base/base_export.h"
#include "base/bits.h"
#include "base/check_op.h"
#include "base/compiler_specific.h"
#include "base/no_destructor.h"
#include "base/notreached.h"
#include "base/partition_alloc_buildflags.h"
#include "base/rand_util.h"
#include "base/stl_util.h"
#include "base/synchronization/lock.h"
#include "base/sys_byteorder.h"
#include "build/build_config.h"
#include "build/buildflag.h"

#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
#include <stdlib.h>
#endif

// We use this to make MEMORY_TOOL_REPLACES_ALLOCATOR behave the same for max
// size as other alloc code.
#define CHECK_MAX_SIZE_OR_RETURN_NULLPTR(size, flags) \
  if (size > kMaxDirectMapped) {                      \
    if (flags & PartitionAllocReturnNull) {           \
      return nullptr;                                 \
    }                                                 \
    PA_CHECK(false);                                  \
  }

namespace base {

typedef void (*OomFunction)(size_t);

// PartitionAlloc supports setting hooks to observe allocations/frees as they
// occur as well as 'override' hooks that allow overriding those operations.
class BASE_EXPORT PartitionAllocHooks {
 public:
  // Log allocation and free events.
  typedef void AllocationObserverHook(void* address,
                                      size_t size,
                                      const char* type_name);
  typedef void FreeObserverHook(void* address);

  // If it returns true, the allocation has been overridden with the pointer in
  // *out.
  typedef bool AllocationOverrideHook(void** out,
                                      int flags,
                                      size_t size,
                                      const char* type_name);
  // If it returns true, then the allocation was overridden and has been freed.
  typedef bool FreeOverrideHook(void* address);
  // If it returns true, the underlying allocation is overridden and *out holds
  // the size of the underlying allocation.
  typedef bool ReallocOverrideHook(size_t* out, void* address);

  // To unhook, call Set*Hooks with nullptrs.
  static void SetObserverHooks(AllocationObserverHook* alloc_hook,
                               FreeObserverHook* free_hook);
  static void SetOverrideHooks(AllocationOverrideHook* alloc_hook,
                               FreeOverrideHook* free_hook,
                               ReallocOverrideHook realloc_hook);

  // Helper method to check whether hooks are enabled. This is an optimization
  // so that if a function needs to call observer and override hooks in two
  // different places this value can be cached and only loaded once.
  static bool AreHooksEnabled() {
    return hooks_enabled_.load(std::memory_order_relaxed);
  }

  static void AllocationObserverHookIfEnabled(void* address,
                                              size_t size,
                                              const char* type_name);
  static bool AllocationOverrideHookIfEnabled(void** out,
                                              int flags,
                                              size_t size,
                                              const char* type_name);

  static void FreeObserverHookIfEnabled(void* address);
  static bool FreeOverrideHookIfEnabled(void* address);

  static void ReallocObserverHookIfEnabled(void* old_address,
                                           void* new_address,
                                           size_t size,
                                           const char* type_name);
  static bool ReallocOverrideHookIfEnabled(size_t* out, void* address);

 private:
  // Single bool that is used to indicate whether observer or allocation hooks
  // are set to reduce the numbers of loads required to check whether hooking is
  // enabled.
  static std::atomic<bool> hooks_enabled_;

  // Lock used to synchronize Set*Hooks calls.
  static std::atomic<AllocationObserverHook*> allocation_observer_hook_;
  static std::atomic<FreeObserverHook*> free_observer_hook_;

  static std::atomic<AllocationOverrideHook*> allocation_override_hook_;
  static std::atomic<FreeOverrideHook*> free_override_hook_;
  static std::atomic<ReallocOverrideHook*> realloc_override_hook_;
};

namespace internal {

ALWAYS_INLINE void* PartitionPointerAdjustSubtract(bool allow_extras,
                                                   void* ptr) {
  if (allow_extras) {
    ptr = PartitionTagPointerAdjustSubtract(ptr);
    ptr = PartitionCookiePointerAdjustSubtract(ptr);
  }
  return ptr;
}

ALWAYS_INLINE void* PartitionPointerAdjustAdd(bool allow_extras, void* ptr) {
  if (allow_extras) {
    ptr = PartitionTagPointerAdjustAdd(ptr);
    ptr = PartitionCookiePointerAdjustAdd(ptr);
  }
  return ptr;
}

ALWAYS_INLINE size_t PartitionSizeAdjustAdd(bool allow_extras, size_t size) {
  if (allow_extras) {
    size = PartitionTagSizeAdjustAdd(size);
    size = PartitionCookieSizeAdjustAdd(size);
  }
  return size;
}

ALWAYS_INLINE size_t PartitionSizeAdjustSubtract(bool allow_extras,
                                                 size_t size) {
  if (allow_extras) {
    size = PartitionTagSizeAdjustSubtract(size);
    size = PartitionCookieSizeAdjustSubtract(size);
  }
  return size;
}

template <bool thread_safe>
class LOCKABLE MaybeSpinLock {
 public:
  void Lock() EXCLUSIVE_LOCK_FUNCTION() {}
  void Unlock() UNLOCK_FUNCTION() {}
  void AssertAcquired() const ASSERT_EXCLUSIVE_LOCK() {}
};

template <bool thread_safe>
class SCOPED_LOCKABLE ScopedGuard {
 public:
  explicit ScopedGuard(MaybeSpinLock<thread_safe>& lock)
      EXCLUSIVE_LOCK_FUNCTION(lock)
      : lock_(lock) {
    lock_.Lock();
  }
  ~ScopedGuard() UNLOCK_FUNCTION() { lock_.Unlock(); }

 private:
  MaybeSpinLock<thread_safe>& lock_;
};

#if DCHECK_IS_ON()
template <>
class LOCKABLE MaybeSpinLock<ThreadSafe> {
 public:
  MaybeSpinLock() : lock_() {}
  void Lock() EXCLUSIVE_LOCK_FUNCTION() {
#if BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
    // When PartitionAlloc is malloc(), it can easily become reentrant. For
    // instance, a DCHECK() triggers in external code (such as
    // base::Lock). DCHECK() error message formatting allocates, which triggers
    // PartitionAlloc, and then we get reentrancy, and in this case infinite
    // recursion.
    //
    // To avoid that, crash quickly when the code becomes reentrant.
    PlatformThreadRef current_thread = PlatformThread::CurrentRef();
    if (!lock_->Try()) {
      // The lock wasn't free when we tried to acquire it. This can be because
      // another thread or *this* thread was holding it.
      //
      // If it's this thread holding it, then it cannot have become free in the
      // meantime, and the current value of |owning_thread_ref_| is valid, as it
      // was set by this thread. Assuming that writes to |owning_thread_ref_|
      // are atomic, then if it's us, we are trying to recursively acquire a
      // non-recursive lock.
      //
      // Note that we don't rely on a DCHECK() in base::Lock(), as it would
      // itself allocate. Meaning that without this code, a reentrancy issue
      // hangs on Linux.
      if (UNLIKELY(TS_UNCHECKED_READ(owning_thread_ref_.load(
                       std::memory_order_relaxed)) == current_thread)) {
        // Trying to acquire lock while it's held by this thread: reentrancy
        // issue.
        IMMEDIATE_CRASH();
      }
      lock_->Acquire();
    }
    owning_thread_ref_.store(current_thread, std::memory_order_relaxed);
#else
    lock_->Acquire();
#endif
  }

  void Unlock() UNLOCK_FUNCTION() {
#if BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
    owning_thread_ref_.store(PlatformThreadRef(), std::memory_order_relaxed);
#endif
    lock_->Release();
  }
  void AssertAcquired() const ASSERT_EXCLUSIVE_LOCK() {
    lock_->AssertAcquired();
  }

 private:
  // NoDestructor to avoid issues with the "static destruction order fiasco".
  //
  // This also means that for DCHECK_IS_ON() builds we leak a lock when a
  // partition is destructed. This will in practice only show in some tests, as
  // partitons are not destructed in regular use. In addition, on most
  // platforms, base::Lock doesn't allocate memory and neither does the OS
  // library, and the destructor is a no-op.
  base::NoDestructor<base::Lock> lock_;

#if BUILDFLAG(USE_PARTITION_ALLOC_AS_MALLOC)
  std::atomic<PlatformThreadRef> owning_thread_ref_ GUARDED_BY(lock_);
#endif
};

#else
template <>
class LOCKABLE MaybeSpinLock<ThreadSafe> {
 public:
  void Lock() EXCLUSIVE_LOCK_FUNCTION() { lock_.lock(); }
  void Unlock() UNLOCK_FUNCTION() { lock_.unlock(); }
  void AssertAcquired() const ASSERT_EXCLUSIVE_LOCK() {
    // Not supported by subtle::SpinLock.
  }

 private:
  subtle::SpinLock lock_;
};
#endif  // DCHECK_IS_ON()

// An "extent" is a span of consecutive superpages. We link to the partition's
// next extent (if there is one) to the very start of a superpage's metadata
// area.
template <bool thread_safe>
struct PartitionSuperPageExtentEntry {
  PartitionRoot<thread_safe>* root;
  char* super_page_base;
  char* super_pages_end;
  PartitionSuperPageExtentEntry<thread_safe>* next;
};
static_assert(
    sizeof(PartitionSuperPageExtentEntry<ThreadSafe>) <= kPageMetadataSize,
    "PartitionSuperPageExtentEntry must be able to fit in a metadata slot");

// g_oom_handling_function is invoked when PartitionAlloc hits OutOfMemory.
static OomFunction g_oom_handling_function = nullptr;

}  // namespace internal

class PartitionStatsDumper;

enum PartitionPurgeFlags {
  // Decommitting the ring list of empty pages is reasonably fast.
  PartitionPurgeDecommitEmptyPages = 1 << 0,
  // Discarding unused system pages is slower, because it involves walking all
  // freelists in all active partition pages of all buckets >= system page
  // size. It often frees a similar amount of memory to decommitting the empty
  // pages, though.
  PartitionPurgeDiscardUnusedSystemPages = 1 << 1,
};

// Struct used to retrieve total memory usage of a partition. Used by
// PartitionStatsDumper implementation.
struct PartitionMemoryStats {
  size_t total_mmapped_bytes;    // Total bytes mmaped from the system.
  size_t total_committed_bytes;  // Total size of commmitted pages.
  size_t total_resident_bytes;   // Total bytes provisioned by the partition.
  size_t total_active_bytes;     // Total active bytes in the partition.
  size_t total_decommittable_bytes;  // Total bytes that could be decommitted.
  size_t total_discardable_bytes;    // Total bytes that could be discarded.
};

// Struct used to retrieve memory statistics about a partition bucket. Used by
// PartitionStatsDumper implementation.
struct PartitionBucketMemoryStats {
  bool is_valid;       // Used to check if the stats is valid.
  bool is_direct_map;  // True if this is a direct mapping; size will not be
                       // unique.
  uint32_t bucket_slot_size;     // The size of the slot in bytes.
  uint32_t allocated_page_size;  // Total size the partition page allocated from
                                 // the system.
  uint32_t active_bytes;         // Total active bytes used in the bucket.
  uint32_t resident_bytes;       // Total bytes provisioned in the bucket.
  uint32_t decommittable_bytes;  // Total bytes that could be decommitted.
  uint32_t discardable_bytes;    // Total bytes that could be discarded.
  uint32_t num_full_pages;       // Number of pages with all slots allocated.
  uint32_t num_active_pages;     // Number of pages that have at least one
                                 // provisioned slot.
  uint32_t num_empty_pages;      // Number of pages that are empty
                                 // but not decommitted.
  uint32_t num_decommitted_pages;  // Number of pages that are empty
                                   // and decommitted.
};

// Interface that is passed to PartitionDumpStats and
// PartitionDumpStats for using the memory statistics.
class BASE_EXPORT PartitionStatsDumper {
 public:
  // Called to dump total memory used by partition, once per partition.
  virtual void PartitionDumpTotals(const char* partition_name,
                                   const PartitionMemoryStats*) = 0;

  // Called to dump stats about buckets, for each bucket.
  virtual void PartitionsDumpBucketStats(const char* partition_name,
                                         const PartitionBucketMemoryStats*) = 0;
};

// Never instantiate a PartitionRoot directly, instead use
// PartitionAllocator.
template <bool thread_safe>
struct BASE_EXPORT PartitionRoot {
  using Page = internal::PartitionPage<thread_safe>;
  using Bucket = internal::PartitionBucket<thread_safe>;
  using SuperPageExtentEntry =
      internal::PartitionSuperPageExtentEntry<thread_safe>;
  using DirectMapExtent = internal::PartitionDirectMapExtent<thread_safe>;
  using ScopedGuard = internal::ScopedGuard<thread_safe>;

  internal::MaybeSpinLock<thread_safe> lock_;
  // Invariant: total_size_of_committed_pages <=
  //                total_size_of_super_pages +
  //                total_size_of_direct_mapped_pages.
  size_t total_size_of_committed_pages = 0;
  size_t total_size_of_super_pages = 0;
  size_t total_size_of_direct_mapped_pages = 0;
  // TODO(bartekn): Consider size of added extras (cookies and/or tag, or
  // nothing) instead of true|false, so that we can just add or subtract the
  // size instead of having an if branch on the hot paths.
  bool allow_extras;
  bool initialized = false;
  char* next_super_page = nullptr;
  char* next_partition_page = nullptr;
  char* next_partition_page_end = nullptr;
  SuperPageExtentEntry* current_extent = nullptr;
  SuperPageExtentEntry* first_extent = nullptr;
  DirectMapExtent* direct_map_list = nullptr;
  Page* global_empty_page_ring[kMaxFreeableSpans] = {};
  int16_t global_empty_page_ring_index = 0;
  uintptr_t inverted_self = 0;
#if ENABLE_TAG_FOR_CHECKED_PTR2 || ENABLE_TAG_FOR_MTE_CHECKED_PTR
  internal::PartitionTag current_partition_tag = 0;
#endif
#if ENABLE_TAG_FOR_MTE_CHECKED_PTR
  char* next_tag_bitmap_page = nullptr;
#endif

  // Some pre-computed constants.
  size_t order_index_shifts[kBitsPerSizeT + 1] = {};
  size_t order_sub_index_masks[kBitsPerSizeT + 1] = {};
  // The bucket lookup table lets us map a size_t to a bucket quickly.
  // The trailing +1 caters for the overflow case for very large allocation
  // sizes.  It is one flat array instead of a 2D array because in the 2D
  // world, we'd need to index array[blah][max+1] which risks undefined
  // behavior.
  Bucket* bucket_lookups[((kBitsPerSizeT + 1) * kNumBucketsPerOrder) + 1] = {};
  Bucket buckets[kNumBuckets] = {};

  PartitionRoot() = default;
  explicit PartitionRoot(bool enable_tag_pointers) {
    Init(enable_tag_pointers);
  }
  ~PartitionRoot() = default;

  // Public API
  //
  // Allocates out of the given bucket. Properly, this function should probably
  // be in PartitionBucket, but because the implementation needs to be inlined
  // for performance, and because it needs to inspect PartitionPage,
  // it becomes impossible to have it in PartitionBucket as this causes a
  // cyclical dependency on PartitionPage function implementations.
  //
  // Moving it a layer lower couples PartitionRoot and PartitionBucket, but
  // preserves the layering of the includes.
  void Init(bool enforce_alignment);

  ALWAYS_INLINE static bool IsValidPage(Page* page);
  ALWAYS_INLINE static PartitionRoot* FromPage(Page* page);

  ALWAYS_INLINE void IncreaseCommittedPages(size_t len);
  ALWAYS_INLINE void DecreaseCommittedPages(size_t len);
  ALWAYS_INLINE void DecommitSystemPages(void* address, size_t length)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);
  ALWAYS_INLINE void RecommitSystemPages(void* address, size_t length)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);

  NOINLINE void OutOfMemory(size_t size);

  // Returns a pointer aligned on |alignement|, or nullptr.
  //
  // |alignment| has to be a power of two and a multiple of sizeof(void*) (as in
  // posix_memalign() for POSIX systems). The returned pointer may include
  // padding, and can be passed to |Free()| later.
  //
  // NOTE: Doesn't work when DCHECK_IS_ON(), as it is incompatible with cookies.
  ALWAYS_INLINE void* AlignedAlloc(size_t alignment, size_t size);

  ALWAYS_INLINE void* Alloc(size_t size, const char* type_name);
  ALWAYS_INLINE void* AllocFlags(int flags, size_t size, const char* type_name);

  ALWAYS_INLINE void* Realloc(void* ptr,
                              size_t new_size,
                              const char* type_name);
  // Overload that may return nullptr if reallocation isn't possible. In this
  // case, |ptr| remains valid.
  ALWAYS_INLINE void* TryRealloc(void* ptr,
                                 size_t new_size,
                                 const char* type_name);
  NOINLINE void* ReallocFlags(int flags,
                              void* ptr,
                              size_t new_size,
                              const char* type_name);
  ALWAYS_INLINE static void Free(void* ptr);

  ALWAYS_INLINE static size_t GetSizeFromPointer(void* ptr);
  ALWAYS_INLINE size_t GetSize(void* ptr) const;
  ALWAYS_INLINE size_t ActualSize(size_t size);

  // Frees memory from this partition, if possible, by decommitting pages.
  // |flags| is an OR of base::PartitionPurgeFlags.
  void PurgeMemory(int flags);

  void DumpStats(const char* partition_name,
                 bool is_light_dump,
                 PartitionStatsDumper* partition_stats_dumper);

  internal::PartitionBucket<thread_safe>* SizeToBucket(size_t size) const;

 private:
  ALWAYS_INLINE void* AllocFromBucket(Bucket* bucket, int flags, size_t size)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);
  bool ReallocDirectMappedInPlace(internal::PartitionPage<thread_safe>* page,
                                  size_t raw_size)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);
  void DecommitEmptyPages() EXCLUSIVE_LOCKS_REQUIRED(lock_);

  ALWAYS_INLINE internal::PartitionTag GetNewPartitionTag()
      EXCLUSIVE_LOCKS_REQUIRED(lock_) {
#if ENABLE_TAG_FOR_CHECKED_PTR2 || ENABLE_TAG_FOR_MTE_CHECKED_PTR
    ++current_partition_tag;
    current_partition_tag += !current_partition_tag;  // Avoid 0.
    return current_partition_tag;
#else
    return 0;
#endif
  }
};

template <bool thread_safe>
ALWAYS_INLINE void* PartitionRoot<thread_safe>::AllocFromBucket(Bucket* bucket,
                                                                int flags,
                                                                size_t size) {
  bool zero_fill = flags & PartitionAllocZeroFill;
  bool is_already_zeroed = false;

  Page* page = bucket->active_pages_head;
  // Check that this page is neither full nor freed.
  PA_DCHECK(page);
  PA_DCHECK(page->num_allocated_slots >= 0);
  size_t new_slot_size = bucket->slot_size;

  void* ret = page->freelist_head;
  if (LIKELY(ret)) {
    // If these DCHECKs fire, you probably corrupted memory. TODO(palmer): See
    // if we can afford to make these CHECKs.
    PA_DCHECK(IsValidPage(page));

    // All large allocations must go through the slow path to correctly update
    // the size metadata.
    PA_DCHECK(page->get_raw_size() == 0);
    internal::PartitionFreelistEntry* new_head =
        internal::EncodedPartitionFreelistEntry::Decode(
            page->freelist_head->next);
    page->freelist_head = new_head;
    page->num_allocated_slots++;

    PA_DCHECK(page->bucket == bucket);
  } else {
    ret = bucket->SlowPathAlloc(this, flags, size, &is_already_zeroed);
    // TODO(palmer): See if we can afford to make this a CHECK.
    PA_DCHECK(!ret || IsValidPage(Page::FromPointer(ret)));

    if (UNLIKELY(!ret))
      return nullptr;

    page = Page::FromPointer(ret);
    if (UNLIKELY(page->get_raw_size())) {
      PA_DCHECK(page->get_raw_size() == size);
      new_slot_size = size;
    } else {
      new_slot_size = page->bucket->slot_size;
    }
  }
  // Layout inside the slot: |[tag]|cookie|object|[empty]|cookie|
  //                                      <--a--->
  //                                      <------b------->
  //                         <-----------------c---------------->
  //   a: size
  //   b: size_with_no_extras
  //   c: new_slot_size
  // Note, empty space occurs if the slot size is larger than needed to
  // accommodate the request.
  // The tag may or may not exist in the slot, depending on CheckedPtr
  // implementation.
  size_t size_with_no_extras =
      internal::PartitionSizeAdjustSubtract(allow_extras, new_slot_size);
  // The value given to the application is just after the tag and cookie.
  ret = internal::PartitionPointerAdjustAdd(allow_extras, ret);

#if DCHECK_IS_ON()
  // Surround the region with 2 cookies.
  if (allow_extras) {
    char* char_ret = static_cast<char*>(ret);
    internal::PartitionCookieWriteValue(char_ret - internal::kCookieSize);
    internal::PartitionCookieWriteValue(char_ret + size_with_no_extras);
  }
#endif

  // Fill the region kUninitializedByte (on debug builds, if not requested to 0)
  // or 0 (if requested and not 0 already).
  if (!zero_fill) {
#if DCHECK_IS_ON()
    memset(ret, kUninitializedByte, size_with_no_extras);
#endif
  } else if (!is_already_zeroed) {
    memset(ret, 0, size_with_no_extras);
  }

  if (allow_extras && !bucket->is_direct_mapped()) {
    size_t slot_size_with_no_extras = internal::PartitionSizeAdjustSubtract(
        allow_extras, page->bucket->slot_size);
    internal::PartitionTagSetValue(ret, slot_size_with_no_extras,
                                   GetNewPartitionTag());
  }

  return ret;
}

// static
template <bool thread_safe>
ALWAYS_INLINE void PartitionRoot<thread_safe>::Free(void* ptr) {
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  free(ptr);
#else
  if (UNLIKELY(!ptr))
    return;

  if (PartitionAllocHooks::AreHooksEnabled()) {
    PartitionAllocHooks::FreeObserverHookIfEnabled(ptr);
    if (PartitionAllocHooks::FreeOverrideHookIfEnabled(ptr))
      return;
  }
  // No check as the pointer hasn't been adjusted yet.
  Page* page = Page::FromPointerNoAlignmentCheck(ptr);
  // TODO(palmer): See if we can afford to make this a CHECK.
  PA_DCHECK(IsValidPage(page));
  auto* root = PartitionRoot<thread_safe>::FromPage(page);
  if (root->allow_extras && !page->bucket->is_direct_mapped()) {
    size_t size_with_no_extras = internal::PartitionSizeAdjustSubtract(
        root->allow_extras, page->bucket->slot_size);
    // TODO(tasak): clear partition tag. Temporarily set the tag to be 0.
    internal::PartitionTagClearValue(ptr, size_with_no_extras);
  }
  ptr = internal::PartitionPointerAdjustSubtract(root->allow_extras, ptr);
  internal::DeferredUnmap deferred_unmap;
  {
    ScopedGuard guard{root->lock_};
    deferred_unmap = page->Free(ptr);
  }
  deferred_unmap.Run();
#endif
}

// static
template <bool thread_safe>
ALWAYS_INLINE bool PartitionRoot<thread_safe>::IsValidPage(Page* page) {
  PartitionRoot* root = FromPage(page);
  return root->inverted_self == ~reinterpret_cast<uintptr_t>(root);
}

template <bool thread_safe>
ALWAYS_INLINE PartitionRoot<thread_safe>* PartitionRoot<thread_safe>::FromPage(
    Page* page) {
  auto* extent_entry = reinterpret_cast<SuperPageExtentEntry*>(
      reinterpret_cast<uintptr_t>(page) & kSystemPageBaseMask);
  return extent_entry->root;
}

template <bool thread_safe>
ALWAYS_INLINE void PartitionRoot<thread_safe>::IncreaseCommittedPages(
    size_t len) {
  total_size_of_committed_pages += len;
  PA_DCHECK(total_size_of_committed_pages <=
            total_size_of_super_pages + total_size_of_direct_mapped_pages);
}

template <bool thread_safe>
ALWAYS_INLINE void PartitionRoot<thread_safe>::DecreaseCommittedPages(
    size_t len) {
  total_size_of_committed_pages -= len;
  PA_DCHECK(total_size_of_committed_pages <=
            total_size_of_super_pages + total_size_of_direct_mapped_pages);
}

template <bool thread_safe>
ALWAYS_INLINE void PartitionRoot<thread_safe>::DecommitSystemPages(
    void* address,
    size_t length) {
  ::base::DecommitSystemPages(address, length);
  DecreaseCommittedPages(length);
}

template <bool thread_safe>
ALWAYS_INLINE void PartitionRoot<thread_safe>::RecommitSystemPages(
    void* address,
    size_t length) {
  PA_CHECK(::base::RecommitSystemPages(address, length, PageReadWrite));
  IncreaseCommittedPages(length);
}

BASE_EXPORT void PartitionAllocGlobalInit(OomFunction on_out_of_memory);
BASE_EXPORT void PartitionAllocGlobalUninitForTesting();

namespace internal {
// Gets the PartitionPage object for the first partition page of the slot span
// that contains |ptr|. It's used with intention to do obtain the slot size.
// CAUTION! It works well for normal buckets, but for direct-mapped allocations
// it'll only work if |ptr| is in the first partition page of the allocation.
template <bool thread_safe>
ALWAYS_INLINE internal::PartitionPage<thread_safe>*
PartitionAllocGetPageForSize(void* ptr) {
  // No need to lock here. Only |ptr| being freed by another thread could
  // cause trouble, and the caller is responsible for that not happening.
  auto* page =
      internal::PartitionPage<thread_safe>::FromPointerNoAlignmentCheck(ptr);
  // This PA_DCHECK has been temporarily commented out, because
  // CheckedPtr2OrMTEImpl calls ThreadSafe variant of
  // PartitionAllocGetSlotOffset even in the NotThreadSafe case. Everything
  // seems to work, except IsValidPage is failing (PartitionRoot's fields are
  // laid out differently between variants).
  // TODO(bartekn): Uncomment once we figure out thread-safety variant mismatch.
  // TODO(palmer): See if we can afford to make this a CHECK.
  //  PA_DCHECK(PartitionRoot<thread_safe>::IsValidPage(page));
  return page;
}
}  // namespace internal

// static
template <bool thread_safe>
ALWAYS_INLINE size_t PartitionRoot<thread_safe>::GetSizeFromPointer(void* ptr) {
  Page* page = Page::FromPointerNoAlignmentCheck(ptr);
  auto* root = PartitionRoot<thread_safe>::FromPage(page);
  return root->GetSize(ptr);
}

// Gets the size of the allocated slot that contains |ptr|, adjusted for cookie
// (if any).
// CAUTION! For direct-mapped allocation, |ptr| has to be within the first
// partition page.
template <bool thread_safe>
ALWAYS_INLINE size_t PartitionRoot<thread_safe>::GetSize(void* ptr) const {
  ptr = internal::PartitionPointerAdjustSubtract(allow_extras, ptr);
  auto* page = internal::PartitionAllocGetPageForSize<thread_safe>(ptr);
  size_t size = internal::PartitionSizeAdjustSubtract(allow_extras,
                                                      page->bucket->slot_size);
  return size;
}

// This file may end up getting included even when PartitionAlloc isn't used,
// but the .cc file won't be linked. Exclude the code that relies on it.
#if BUILDFLAG(USE_PARTITION_ALLOC)

namespace internal {
// Avoid including partition_address_space.h from this .h file, by moving the
// call to IfManagedByPartitionAllocNormalBuckets into the .cc file.
#if DCHECK_IS_ON()
BASE_EXPORT void DCheckIfManagedByPartitionAllocNormalBuckets(const void* ptr);
#else
ALWAYS_INLINE void DCheckIfManagedByPartitionAllocNormalBuckets(const void*) {}
#endif
}  // namespace internal

// Gets the offset from the beginning of the allocated slot, adjusted for cookie
// (if any).
// CAUTION! Use only for normal buckets. Using on direct-mapped allocations may
// lead to undefined behavior.
template <bool thread_safe>
ALWAYS_INLINE size_t PartitionAllocGetSlotOffset(void* ptr) {
  internal::DCheckIfManagedByPartitionAllocNormalBuckets(ptr);
  // The only allocations that don't use tag are allocated outside of GigaCage,
  // hence we'd never get here in the use_tag=false case.
  // TODO(bartekn): Add a DCHECK(page->root->allow_extras) to assert this, once
  // we figure out the thread-safety variant mismatch problem (see the comment
  // in PartitionAllocGetPageForSize for the problem description).
  ptr = internal::PartitionPointerAdjustSubtract(true /* use_tag */, ptr);
  auto* page = internal::PartitionAllocGetPageForSize<thread_safe>(ptr);
  size_t slot_size = page->bucket->slot_size;

  // Get the offset from the beginning of the slot span.
  uintptr_t ptr_addr = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t slot_span_start = reinterpret_cast<uintptr_t>(
      internal::PartitionPage<thread_safe>::ToPointer(page));
  size_t offset_in_slot_span = ptr_addr - slot_span_start;
  // Knowing that slots are tightly packed in a slot span, calculate an offset
  // within a slot using simple % operation.
  // TODO(bartekn): Try to replace % with multiplication&shift magic.
  size_t offset_in_slot = offset_in_slot_span % slot_size;
  return offset_in_slot;
}

#endif  // BUILDFLAG(USE_PARTITION_ALLOC)

template <bool thread_safe>
ALWAYS_INLINE internal::PartitionBucket<thread_safe>*
PartitionRoot<thread_safe>::SizeToBucket(size_t size) const {
  size_t order = kBitsPerSizeT - bits::CountLeadingZeroBitsSizeT(size);
  // The order index is simply the next few bits after the most significant bit.
  size_t order_index =
      (size >> order_index_shifts[order]) & (kNumBucketsPerOrder - 1);
  // And if the remaining bits are non-zero we must bump the bucket up.
  size_t sub_order_index = size & order_sub_index_masks[order];
  Bucket* bucket = bucket_lookups[(order << kNumBucketsPerOrderBits) +
                                  order_index + !!sub_order_index];
  PA_CHECK(bucket);
  PA_DCHECK(!bucket->slot_size || bucket->slot_size >= size);
  PA_DCHECK(!(bucket->slot_size % kSmallestBucket));
  return bucket;
}

template <bool thread_safe>
ALWAYS_INLINE void* PartitionRoot<thread_safe>::AllocFlags(
    int flags,
    size_t size,
    const char* type_name) {
  PA_DCHECK(flags < PartitionAllocLastFlag << 1);

#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  CHECK_MAX_SIZE_OR_RETURN_NULLPTR(size, flags);
  const bool zero_fill = flags & PartitionAllocZeroFill;
  void* result = zero_fill ? calloc(1, size) : malloc(size);
  PA_CHECK(result || flags & PartitionAllocReturnNull);
  return result;
#else
  PA_DCHECK(initialized);
  void* result;
  const bool hooks_enabled = PartitionAllocHooks::AreHooksEnabled();
  if (UNLIKELY(hooks_enabled)) {
    if (PartitionAllocHooks::AllocationOverrideHookIfEnabled(&result, flags,
                                                             size, type_name)) {
      PartitionAllocHooks::AllocationObserverHookIfEnabled(result, size,
                                                           type_name);
      return result;
    }
  }
  size_t requested_size = size;
  size = internal::PartitionSizeAdjustAdd(allow_extras, size);
  PA_CHECK(size >= requested_size);  // check for overflows
  auto* bucket = SizeToBucket(size);
  PA_DCHECK(bucket);
  {
    internal::ScopedGuard<thread_safe> guard{lock_};
    result = AllocFromBucket(bucket, flags, size);
  }
  if (UNLIKELY(hooks_enabled)) {
    PartitionAllocHooks::AllocationObserverHookIfEnabled(result, requested_size,
                                                         type_name);
  }

  return result;
#endif
}

template <bool thread_safe>
ALWAYS_INLINE void* PartitionRoot<thread_safe>::AlignedAlloc(size_t alignment,
                                                             size_t size) {
  // Aligned allocation support relies on the natural alignment guarantees of
  // PartitionAlloc. Since cookies and tags are layered on top of
  // PartitionAlloc, they change the guarantees. As a consequence, forbid both.
  PA_DCHECK(!allow_extras);
  // This is mandated by |posix_memalign()|, so should never fire.
  PA_CHECK(base::bits::IsPowerOfTwo(alignment));

  void* ptr;
  // Handle cases such as size = 16, alignment = 64.
  // Wastes memory when a large alignment is requested with a small size, but
  // this is hard to avoid, and should not be too common.
  if (UNLIKELY(size < alignment)) {
    ptr = Alloc(alignment, "");
  } else {
    // PartitionAlloc only guarantees alignment for power-of-two sized
    // allocations. To make sure this applies here, round up the allocation
    // size.
    size_t size_rounded_up =
        static_cast<size_t>(1)
        << (sizeof(size_t) * 8 - base::bits::CountLeadingZeroBits(size - 1));
    ptr = Alloc(size_rounded_up, "");
  }

  // |alignment| is a power of two, but the compiler doesn't necessarily know
  // that. A regular % operation is very slow, make sure to use the equivalent,
  // faster form.
  PA_CHECK(!(reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)));

  return ptr;
}

template <bool thread_safe>
ALWAYS_INLINE void* PartitionRoot<thread_safe>::Alloc(size_t size,
                                                      const char* type_name) {
  return AllocFlags(0, size, type_name);
}

template <bool thread_safe>
ALWAYS_INLINE void* PartitionRoot<thread_safe>::Realloc(void* ptr,
                                                        size_t new_size,
                                                        const char* type_name) {
  return ReallocFlags(0, ptr, new_size, type_name);
}

template <bool thread_safe>
ALWAYS_INLINE void* PartitionRoot<thread_safe>::TryRealloc(
    void* ptr,
    size_t new_size,
    const char* type_name) {
  return ReallocFlags(PartitionAllocReturnNull, ptr, new_size, type_name);
}

template <bool thread_safe>
ALWAYS_INLINE size_t PartitionRoot<thread_safe>::ActualSize(size_t size) {
#if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)
  return size;
#else
  PA_DCHECK(PartitionRoot<thread_safe>::initialized);
  size = internal::PartitionSizeAdjustAdd(allow_extras, size);
  auto* bucket = SizeToBucket(size);
  if (LIKELY(!bucket->is_direct_mapped())) {
    size = bucket->slot_size;
  } else if (size > kMaxDirectMapped) {
    // Too large to allocate => return the size unchanged.
  } else {
    size = Bucket::get_direct_map_size(size);
  }
  size = internal::PartitionSizeAdjustSubtract(allow_extras, size);
  return size;
#endif
}

enum class PartitionAllocatorAlignment {
  // By default all allocations will be aligned to 8B (16B if
  // BUILDFLAG_INTERNAL_USE_PARTITION_ALLOC_AS_MALLOC is true).
  kRegular,

  // In addition to the above alignment enforcement, this option allows using
  // AlignedAlloc() which can align at a larger boundary.
  // This option comes at a cost of disallowing cookies on Debug builds and tags
  // for CheckedPtr. It also causes all allocations to go outside of GigaCage,
  // so that CheckedPtr can easily tell if a pointer comes with a tag or not.
  kAlignedAlloc,
};

namespace internal {
template <bool thread_safe>
struct BASE_EXPORT PartitionAllocator {
  PartitionAllocator() = default;
  ~PartitionAllocator();

  void init(PartitionAllocatorAlignment alignment =
                PartitionAllocatorAlignment::kRegular);
  ALWAYS_INLINE PartitionRoot<thread_safe>* root() { return &partition_root_; }

 private:
  PartitionRoot<thread_safe> partition_root_;
};

}  // namespace internal

using PartitionAllocator = internal::PartitionAllocator<internal::ThreadSafe>;
using ThreadUnsafePartitionAllocator =
    internal::PartitionAllocator<internal::NotThreadSafe>;

using ThreadSafePartitionRoot = PartitionRoot<internal::ThreadSafe>;
using ThreadUnsafePartitionRoot = PartitionRoot<internal::NotThreadSafe>;

}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ALLOC_H_
