// #pragma once
//
// #include "cache.h"
//
// class CacheDonuts : public Cache
// {
// public:
//    CacheDonuts(const String& name,
//                const String& cfgname,
//                core_id_t core_id,
//                UInt32 num_sets,
//                UInt32 associativity,
//                UInt32 cache_block_size,
//                ReplacementPolicy replacement_policy,
//                cache_t cache_type,
//                hash_t hash,
//                FaultInjector* fault_injector,
//                AddressHomeLookup* ahl);
//
//    [[nodiscard]] float getCapacityUsed() const;
//    [[nodiscard]] float getSetCapacityUsed(UInt32 index) const;
//    [[nodiscard]] float getCacheThreshold() const { return m_cache_threshold; }
//    [[nodiscard]] float getCacheSetThreshold() const { return m_cache_set_threshold; }
//
//    [[nodiscard]] static bool isDonutsAndLLC(const String& cfgname);
//
// private:
//    static constexpr float DEFAULT_CACHE_THRESHOLD     = 0.75;
//    static constexpr float DEFAULT_CACHE_SET_THRESHOLD = 1.0;
//
//    float m_cache_threshold;
//    float m_cache_set_threshold;
// };
