#pragma once

#include "log.h"
#include "subsecond_time.h"

#include <concepts>

class SerializableSnapshot
{
public:
   virtual ~SerializableSnapshot() = default;

   virtual String toJson(bool embed) const = 0;
   virtual String toYaml(bool embed) const = 0;

   virtual String asJson() const { return toJson(false); }
   virtual String asYaml() const { return toYaml(false); }

   virtual String toString() const { return asYaml(); }

protected:
   constexpr static int SPACES_PER_INDENT = 2;

   static String indent(const UInt8 level, const UInt8 spaces_per_indent = SPACES_PER_INDENT)
   {
      return String(level * spaces_per_indent, ' ');
   }
};

class Snapshot
{
public:
   SubsecondTime getGlobalTime() const noexcept { return m_global_time; }

protected:
   SubsecondTime m_global_time;

   explicit Snapshot(const SubsecondTime& global_time) :
       m_global_time(global_time) {}
};

class IntervalSnapshot
{
public:
   SubsecondTime getDuration() const noexcept { return m_duration; }

protected:
   SubsecondTime m_duration;

   explicit IntervalSnapshot(const SubsecondTime& duration) :
       m_duration(duration) {}
};

/*
 ==========================================================
 Generic Snapshot
 ==========================================================
 */
template<typename T, typename R>
concept SubtractableTo = requires(const T& a, const T& b) {
   { a - b } -> std::same_as<R>;
};

template<typename R>
class GenericIntervalSnapshot
{
   template<typename T, typename U>
      requires SubtractableTo<T, U>
   friend class GenericSnapshot;

public:
   auto operator->() const noexcept { return &m_info; }

   [[nodiscard]] R getInfo() const noexcept { return m_info; }
   [[nodiscard]] SubsecondTime getDuration() const noexcept { return m_duration; }

protected:
   GenericIntervalSnapshot(const R& info, const SubsecondTime& duration) :
       m_info(info), m_duration(duration) {}

private:
   const R m_info;
   const SubsecondTime m_duration;
};

template<typename T, typename R>
   requires SubtractableTo<T, R>
class GenericSnapshot
{
public:
   GenericSnapshot(const T& info, const SubsecondTime& global_time) :
       m_info(info), m_global_time(global_time) {}

   [[nodiscard]] T getInfo() const noexcept { return m_info; }
   [[nodiscard]] SubsecondTime getGlobalTime() const noexcept { return m_global_time; }

   auto operator-(const GenericSnapshot& rhs)
   {
      LOG_ASSERT_WARNING(m_global_time >= rhs.m_global_time,
                         "Invalid snapshot order: oldest timestamp (%lu) is later than current snapshot timestamp (%lu)",
                         rhs.m_global_time.getNS(), m_global_time.getNS());

      return GenericIntervalSnapshot(m_info - rhs.m_info, m_global_time - rhs.m_global_time);
   }

   auto operator->() const noexcept { return &m_info; }

private:
   const T m_info;
   const SubsecondTime m_global_time;
};
