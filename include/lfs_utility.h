#pragma once

#ifndef _MSC_VER
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

static inline int strcpy_s(char* dest, size_t destsz, const char* src) {
    if (!dest || !src || destsz == 0) return -1;
    size_t len = strlen(src);
    if (len >= destsz) {
        dest[0] = '\0';
        return -1;
    }
    memcpy(dest, src, len + 1);
    return 0;
}
#endif

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(*(a)))
#endif

#if defined(_MSC_VER)
// Bit builtin's make these assumptions when calling _BitScanForward/Reverse
// etc. These assumptions are expected to be true for Win32/Win64 which this
// file supports.
static_assert(sizeof(unsigned long long) == 8, "");
static_assert(sizeof(unsigned long) == 4, "");
static_assert(sizeof(unsigned int) == 4, "");
__forceinline int __builtin_popcount(unsigned int x)
{
    static const unsigned int m1 = 0x55555555;
    static const unsigned int m2 = 0x33333333;
    static const unsigned int m4 = 0x0f0f0f0f;
    static const unsigned int h01 = 0x01010101;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return (x * h01) >> 24;
}
__forceinline int __builtin_popcountl(unsigned long x)
{
    return __builtin_popcount(static_cast<int>(x));
}
__forceinline int __builtin_popcountll(unsigned long long x)
{
    static const unsigned long long m1 = 0x5555555555555555;
    static const unsigned long long m2 = 0x3333333333333333;
    static const unsigned long long m4 = 0x0f0f0f0f0f0f0f0f;
    static const unsigned long long h01 = 0x0101010101010101;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return static_cast<int>((x * h01) >> 56);
}
__forceinline int __builtin_ctzll(unsigned long long mask)
{
    unsigned long where;
#if defined(_WIN64)
    if (_BitScanForward64(&where, mask))
        return static_cast<int>(where);
#elif defined(_WIN32)
    if (_BitScanForward(&where, static_cast<unsigned long>(mask)))
        return static_cast<int>(where);
    if (_BitScanForward(&where, static_cast<unsigned long>(mask >> 32)))
        return static_cast<int>(where + 32);
#else
#error "Implementation of __builtin_ctzll required"
#endif
    return 64;
}
__forceinline int __builtin_ctzl(unsigned long mask)
{
    unsigned long where;
    if (_BitScanForward(&where, mask))
        return static_cast<int>(where);
    return 32;
}
__forceinline int __builtin_ctz(unsigned int mask)
{
    static_assert(sizeof(mask) == 4, "");
    static_assert(sizeof(unsigned long) == 4, "");
    return __builtin_ctzl(static_cast<unsigned long>(mask));
}
__forceinline int __builtin_clzll(unsigned long long mask)
{
    unsigned long where;
#if defined(_WIN64)
    if (_BitScanReverse64(&where, mask))
        return static_cast<int>(63 - where);
#elif defined(_WIN32)
    if (_BitScanReverse(&where, static_cast<unsigned long>(mask >> 32)))
        return static_cast<int>(63 - (where + 32));
    if (_BitScanReverse(&where, static_cast<unsigned long>(mask)))
        return static_cast<int>(63 - where);
#else
#error "Implementation of __builtin_clzll required"
#endif
    return 64;
}
__forceinline int __builtin_clzl(unsigned long mask)
{
    unsigned long where;
    if (_BitScanReverse(&where, mask))
        return static_cast<int>(31 - where);
    return 32;
}
__forceinline int __builtin_clz(unsigned int x)
{
    return __builtin_clzl(x);
}
#endif // _MSC_VER

// Logging functions
#ifndef LFS_TRACE
#ifdef LFS_YES_TRACE
#define LFS_TRACE_(fmt, ...) \
    printf("%s:%d:trace: " fmt "%s\n", __FILE__, __LINE__, __VA_ARGS__)
#define LFS_TRACE(...) LFS_TRACE_(__VA_ARGS__, "")
#else
#define LFS_TRACE(...)
#endif
#endif

#ifndef LFS_DEBUG
#ifndef LFS_NO_DEBUG
#define LFS_DEBUG(fmt, ...) \
    printf("%s:%d:debug: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LFS_DEBUG(...)
#endif
#endif

#ifndef LFS_WARN
#ifndef LFS_NO_WARN
#define LFS_WARN(fmt, ...) \
    printf("%s:%d:warn: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LFS_WARN(...)
#endif
#endif

#ifndef LFS_ERROR
#ifndef LFS_NO_ERROR
#define LFS_ERROR(fmt, ...) \
    printf("%s:%d:error: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LFS_ERROR(...)
#endif
#endif

// Runtime assertions
#ifndef LFS_ASSERT
#ifndef LFS_NO_ASSERT
#define LFS_ASSERT(test) assert(test)
#else
#define LFS_ASSERT(test)
#endif
#endif

// Min/max functions for unsigned 64-bit numbers
constexpr inline uint64_t lfs_max(uint64_t a, uint64_t b) {
    return (a > b) ? a : b;
}

constexpr inline uint64_t lfs_min(uint64_t a, uint64_t b) {
    return (a < b) ? a : b;
}

// Align to nearest multiple of a size
constexpr inline uint64_t lfs_aligndown(uint64_t a, uint64_t alignment) {
    return a - (a % alignment);
}

static inline uint64_t lfs_alignup(uint64_t a, uint64_t alignment) {
    return lfs_aligndown(a + alignment - 1, alignment);
}

// Find the smallest power of 2 greater than or equal to a
constexpr inline uint32_t lfs_npw2_32(uint32_t a) {
#if !defined(LFS_NO_INTRINSICS) && (defined(__GNUC__) || defined(__CC_ARM))
    return 32 - __builtin_clz(a - 1);
#else
    uint32_t r = 0;
    uint32_t s = 0;
    a -= 1;
    s = (a > 0xffff) << 4; a >>= s; r |= s;
    s = (a > 0xff) << 3; a >>= s; r |= s;
    s = (a > 0xf) << 2; a >>= s; r |= s;
    s = (a > 0x3) << 1; a >>= s; r |= s;
    return (r | (a >> 1)) + 1;
#endif
}

inline uint64_t lfs_npw2_64(uint64_t a) {
    if (a == 1) {
        return 1;
    }
    return 64 - __builtin_clzll(a - 1);
}

// Count the number of trailing binary zeros in a
constexpr inline uint32_t lfs_ctz32(uint32_t a) {
#if !defined(LFS_NO_INTRINSICS) && defined(__GNUC__)
    return __builtin_ctz(a);
#else
    return lfs_npw2_32((a & (0 - a)) + 1) - 1;
#endif
}

inline uint64_t lfs_ctz64(uint64_t a) {
    uint64_t result = 0;
    for (result = 0; result < 64; result++) {
        if (a & ((uint64_t)1 << result)) {
            return result;
        }
    }
    return 0;
}

// Count the number of binary ones in a
constexpr inline size_t lfs_popc64(uint64_t V) {
    V -= ((V >> 1) & 0x5555555555555555);
    V = (V & 0x3333333333333333) + ((V >> 2) & 0x3333333333333333);
    return ((V + (V >> 4) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;
}

// Find the sequence comparison of a and b, this is the distance
// between a and b ignoring overflow
constexpr inline int lfs_scmp(uint32_t a, uint32_t b) {
    return (int)(unsigned)(a - b);
}

// Convert between 32-bit/64-bit little-endian and native order
constexpr inline uint32_t lfs_bswap32(uint32_t a) {
    return ((a & 0x000000FFu) << 24) |
           ((a & 0x0000FF00u) << 8)  |
           ((a & 0x00FF0000u) >> 8)  |
           ((a & 0xFF000000u) >> 24);
}

constexpr inline uint64_t lfs_bswap64(uint64_t a) {
    return ((a & 0x00000000000000FFull) << 56) |
           ((a & 0x000000000000FF00ull) << 40) |
           ((a & 0x0000000000FF0000ull) << 24) |
           ((a & 0x00000000FF000000ull) << 8)  |
           ((a & 0x000000FF00000000ull) >> 8)  |
           ((a & 0x0000FF0000000000ull) >> 24) |
           ((a & 0x00FF000000000000ull) >> 40) |
           ((a & 0xFF00000000000000ull) >> 56);
}

#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || \
    (defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN)
constexpr inline uint32_t lfs_fromle32(uint32_t a) { return lfs_bswap32(a); }
constexpr inline uint64_t lfs_fromle64(uint64_t a) { return lfs_bswap64(a); }
constexpr inline uint32_t lfs_tole32(uint32_t a)   { return lfs_bswap32(a); }
constexpr inline uint64_t lfs_tole64(uint64_t a)   { return lfs_bswap64(a); }
constexpr inline uint32_t lfs_frombe32(uint32_t a) { return a; }
constexpr inline uint64_t lfs_frombe64(uint64_t a) { return a; }
constexpr inline uint32_t lfs_tobe32(uint32_t a)   { return a; }
constexpr inline uint64_t lfs_tobe64(uint64_t a)   { return a; }
#else
constexpr inline uint32_t lfs_fromle32(uint32_t a) { return a; }
constexpr inline uint64_t lfs_fromle64(uint64_t a) { return a; }
constexpr inline uint32_t lfs_tole32(uint32_t a)   { return a; }
constexpr inline uint64_t lfs_tole64(uint64_t a)   { return a; }
constexpr inline uint32_t lfs_frombe32(uint32_t a) { return lfs_bswap32(a); }
constexpr inline uint64_t lfs_frombe64(uint64_t a) { return lfs_bswap64(a); }
constexpr inline uint32_t lfs_tobe32(uint32_t a)   { return lfs_bswap32(a); }
constexpr inline uint64_t lfs_tobe64(uint64_t a)   { return lfs_bswap64(a); }
#endif

// operations on block pairs
constexpr inline void lfs_pair_swap(lfs_block_t pair[2]) {
    lfs_block_t t = pair[0];
    pair[0] = pair[1];
    pair[1] = t;
}

constexpr inline bool lfs_pair_isnull(const lfs_block_t pair[2]) {
    return pair[0] == LFS_BLOCK_NULL || pair[1] == LFS_BLOCK_NULL;
}

constexpr inline int lfs_pair_cmp(const lfs_block_t paira[2], const lfs_block_t pairb[2]) {
    return !(paira[0] == pairb[0] || paira[1] == pairb[1] ||
             paira[0] == pairb[1] || paira[1] == pairb[0]);
}

constexpr inline bool lfs_pair_sync(const lfs_block_t paira[2], const lfs_block_t pairb[2]) {
    return (paira[0] == pairb[0] && paira[1] == pairb[1]) ||
           (paira[0] == pairb[1] && paira[1] == pairb[0]);
}

inline void lfs_pair_fromle64(lfs_block_t pair[2]) {
    pair[0] = lfs_fromle64(pair[0]);
    pair[1] = lfs_fromle64(pair[1]);
}

inline void lfs_pair_tole64(lfs_block_t pair[2]) {
    pair[0] = lfs_tole64(pair[0]);
    pair[1] = lfs_tole64(pair[1]);
}

// operations on 32-bit tag values
constexpr inline lfs_tag_t lfs_tag_type1(lfs_tag_t tag) {
    return (tag >> 20) & 0xf00;
}

constexpr inline lfs_tag_t lfs_tag_type2(lfs_tag_t tag) {
    return (tag >> 20) & 0x0ff;
}

constexpr inline lfs_tag_t lfs_tag_type3(lfs_tag_t tag) {
    return (tag >> 20) & 0xfff;
}

constexpr inline lfs_tag_t lfs_tag_id(lfs_tag_t tag) {
    return (tag >> 10) & 0x3ff;
}

constexpr inline lfs_size_t lfs_tag_size(lfs_tag_t tag) {
    return tag & 0x3ff;
}

constexpr inline bool lfs_tag_isvalid(lfs_tag_t tag) {
    return !(tag & 0x80000000);
}

constexpr inline bool lfs_tag_isdelete(lfs_tag_t tag) {
    return ((int32_t)(tag << 22) >> 22) == -1;
}

#define LFS_MKTAG(type, id, size) \
    (((lfs_tag_t)(type) << 20) | ((lfs_tag_t)(id) << 10) | (lfs_tag_t)(size))

#define LFS_MKTAG_IF(cond, type, id, size) \
    ((cond) ? LFS_MKTAG(type, id, size) : LFS_FROM_NOOP)

#define LFS_MKTAG_IF_ELSE(cond, type1, id1, size1, type2, id2, size2) \
    ((cond) ? LFS_MKTAG(type1, id1, size1) : LFS_MKTAG(type2, id2, size2))

// operations on attributes in attribute lists
constexpr inline bool lfs_mattr_isvalid(const lfs_metadata_attribute_t* attr) {
    return lfs_tag_isvalid(attr->tag);
}

// operations on filesystem states
constexpr inline bool lfs_gstate_iszero(const lfs_gstate_t* a) {
    return a->tag == 0 && a->pair[0] == 0 && a->pair[1] == 0;
}

constexpr inline bool lfs_gstate_hasorphans(const lfs_gstate_t* a) {
    return lfs_tag_size(a->tag);
}

constexpr inline uint8_t lfs_gstate_getorphans(const lfs_gstate_t* a) {
    return static_cast<uint8_t>(lfs_tag_size(a->tag));
}

constexpr inline bool lfs_gstate_hasmove(const lfs_gstate_t* a) {
    return lfs_tag_type1(a->tag);
}

constexpr inline bool lfs_gstate_hasmovehere(const lfs_gstate_t* a, const lfs_block_t pair[2]) {
    return lfs_tag_type1(a->tag) && lfs_pair_cmp(a->pair, pair) == 0;
}

inline void lfs_gstate_xor(lfs_gstate_t* a, const lfs_gstate_t* b) {
    a->tag ^= b->tag;
    a->pair[0] ^= b->pair[0];
    a->pair[1] ^= b->pair[1];
}

inline void lfs_gstate_fromle64(lfs_gstate_t* a) {
    a->tag = lfs_fromle32(a->tag);
    a->pair[0] = lfs_fromle64(a->pair[0]);
    a->pair[1] = lfs_fromle64(a->pair[1]);
}

inline void lfs_gstate_tole64(lfs_gstate_t* a) {
    a->tag = lfs_tole32(a->tag);
    a->pair[0] = lfs_tole64(a->pair[0]);
    a->pair[1] = lfs_tole64(a->pair[1]);
}

// CRC32 calculation
uint32_t lfs_crc(uint32_t crc, const void* buffer, size_t size);

// linked list operations
constexpr inline bool lfs_mlist_isopen(lfs_metadata_list_t* head, lfs_metadata_list_t* node) {
    for (lfs_metadata_list_t** p = &head; *p; p = &(*p)->next) {
        if (*p == node) {
            return true;
        }
    }
    return false;
}

inline void lfs_mlist_append(lfs_t* lfs, lfs_metadata_list_t* node) {
    node->next = lfs->metadata_list;
    lfs->metadata_list = node;
}

inline void lfs_mlist_remove(lfs_t* lfs, lfs_metadata_list_t* node) {
    for (lfs_metadata_list_t** p = &lfs->metadata_list; *p; p = &(*p)->next) {
        if (*p == node) {
            *p = (*p)->next;
            break;
        }
    }
}

// disk cache operations
inline void lfs_cache_zero(lfs_t* lfs, lfs_cache_t* cache) {
    cache->block = LFS_BLOCK_NULL;
    cache->offset = 0;
    cache->size = 0;
}

inline void lfs_cache_drop(lfs_t* lfs, lfs_cache_t* cache) {
    cache->block = LFS_BLOCK_NULL;
}

inline void lfs_superblock_tole64(lfs_superblock_t* superblock) {
    superblock->version = lfs_tole32(superblock->version);
    superblock->block_size = lfs_tole64(superblock->block_size);
    superblock->block_count = lfs_tole64(superblock->block_count);
    superblock->name_max_length = lfs_tole64(superblock->name_max_length);
    superblock->file_max_size = lfs_tole64(superblock->file_max_size);
    superblock->attr_max_size = lfs_tole64(superblock->attr_max_size);
}

inline void lfs_superblock_fromle64(lfs_superblock_t* superblock) {
    superblock->version = lfs_fromle32(superblock->version);
    superblock->block_size = lfs_fromle64(superblock->block_size);
    superblock->block_count = lfs_fromle64(superblock->block_count);
    superblock->name_max_length = lfs_fromle64(superblock->name_max_length);
    superblock->file_max_size = lfs_fromle64(superblock->file_max_size);
    superblock->attr_max_size = lfs_fromle64(superblock->attr_max_size);
}