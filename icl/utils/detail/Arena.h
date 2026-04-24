// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

/// \file
/// Page-backed bump allocator for small, trivially-destructible
/// objects.  Trades fine-grained free() for maximum allocation
/// throughput and locality — the entire arena is released in one
/// stroke when its owner destructs.
///
/// Primary consumer: `icl::utils::xml::Document` uses it for
/// `ElementNode` + `AttributeNode` storage (node-pool allocator
/// landed Session 57, −35% parse_large).
///
/// Not currently used in the YAML lib: `yaml::Node` stores its
/// payload by value inside `std::vector<Node>` (sequence) and
/// `std::vector<pair<sv,Node>>` (mapping) — a vector-growth model
/// rather than pointer-based nodes.  Swapping YAML to arena
/// allocation would require reworking `Node::Mapping` /
/// `Node::Sequence` to use pointer-linked storage, which is a
/// bigger refactor than the expected win justifies.  The primitive
/// is kept here so a future rework has a tested hook to reach for.

#pragma once

#include <icl/utils/CompatMacros.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

namespace icl::utils::detail {

  /// Page-backed bump allocator.  One template parameter tunes the
  /// per-page size (default 64 KB — sized so a ~50 KB XML config
  /// fits in a single page).  Any T allocated must be trivially
  /// destructible; this is enforced at the `alloc<T>()` call site
  /// via static_assert.
  ///
  /// Threading: not thread-safe; callers that need concurrent
  /// alloc from multiple threads should serialise externally or
  /// arena-per-thread.
  template<std::size_t PageBytes = 64 * 1024>
  class Arena {
  public:
    Arena(){ m_pages.reserve(4); }
    ~Arena() = default;
    Arena(Arena&&) noexcept = default;
    Arena& operator=(Arena&&) noexcept = default;
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    /// Allocate one default-constructed T in the current page.
    /// T must be trivially destructible — no dtors are ever run;
    /// pages are released wholesale on Arena destruction.
    template<class T>
    T *alloc(){
      static_assert(std::is_trivially_destructible_v<T>,
                    "detail::Arena allocates only trivially-"
                    "destructible types (no dtors are run on release)");
      void *mem = rawAlloc(sizeof(T), alignof(T));
      return ::new (mem) T();
    }

    /// Total bytes reserved across all live pages (not bytes
    /// actually consumed).  Useful for sanity-checking config-
    /// loading working-set size.
    std::size_t pagesReserved() const noexcept {
      return m_pages.size() * PageBytes;
    }

  private:
    struct Page {
      std::unique_ptr<char[]> buf;
    };
    std::vector<Page>  m_pages;
    char              *m_cur = nullptr;   // bump pointer into current page
    char              *m_end = nullptr;   // one-past-end of current page

    void *rawAlloc(std::size_t bytes, std::size_t align){
      std::uintptr_t p = reinterpret_cast<std::uintptr_t>(m_cur);
      std::uintptr_t a = static_cast<std::uintptr_t>(align);
      std::uintptr_t aligned = (p + a - 1) & ~(a - 1);
      char *slot = reinterpret_cast<char *>(aligned);
      if(slot + bytes > m_end){
        newPage(bytes + align);
        p = reinterpret_cast<std::uintptr_t>(m_cur);
        aligned = (p + a - 1) & ~(a - 1);
        slot = reinterpret_cast<char *>(aligned);
      }
      m_cur = slot + bytes;
      return slot;
    }

    void newPage(std::size_t atLeast){
      std::size_t sz = PageBytes;
      if(atLeast > sz) sz = atLeast;
      Page page;
      page.buf.reset(new char[sz]);
      m_pages.push_back(std::move(page));
      m_cur = m_pages.back().buf.get();
      m_end = m_cur + sz;
    }
  };

}  // namespace icl::utils::detail
