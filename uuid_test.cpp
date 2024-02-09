#include <boost/date_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

uint64_t TimestampFromUUID(const boost::uuids::uuid& uuid) {
  static constexpr const int UUID_SIZE = 16;
  static_assert(sizeof(uuid) == UUID_SIZE, "Invalid size of uuid");

  static constexpr const int MS_FROM_100NS_FACTOR = 10000;
  static constexpr const uint64_t OFFSET_FROM_15_10_1582_TO_EPOCH = 122192928000000000;

  /* convert uuid to string for manipulation */
  std::string uuid_str = boost::uuids::to_string(uuid);
  /* store uuid parts in a vector */
  std::vector<std::string> uuid_parts;

  /* split uuid with '-' as delimiter */
  boost::split(uuid_parts, uuid_str, [](char c){return c == '-';});

  /* first part of uuid is time-low
     second part is time-mid
     third part is time high with most significant 4 bits as uuid version
  */
  std::string uuid_timestamp = uuid_parts[2].substr(1) + uuid_parts[1] + uuid_parts[0];
  std::cout << std::endl << "UUID Timestamp : " << uuid_timestamp << std::endl;

  uint64_t timestamp = std::stoul(uuid_timestamp, nullptr, 16);

  return (timestamp - OFFSET_FROM_15_10_1582_TO_EPOCH) / MS_FROM_100NS_FACTOR;
}

int main2() {
  auto gg = boost::uuids::string_generator();
  std::cout << "Time now: " << (boost::posix_time::second_clock::universal_time() - boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))).total_milliseconds() << std::endl;
  auto gen = boost::uuids::string_generator();
  std::cout << "UUID: " << gen("49cbda60-961b-11e8-9854-134d5b3f9cf8") << std::endl;
  std::cout << "Time from UUID: " << TimestampFromUUID(gen("49cbda60-961b-11e8-9854-134d5b3f9cf8")) << std::endl;
  std::cout << "UUID: " << gen("58e0a7d7-eebc-11d8-9669-0800200c9a66") << std::endl;
  std::cout << "Time from UUID: " << TimestampFromUUID(gen("58e0a7d7-eebc-11d8-9669-0800200c9a66")) << std::endl;

  return 0;
}

#include <uuid/uuid.h>

int main() {
    // Declare a uuid_t variable
    uuid_t uuid;
    auto gen = boost::uuids::string_generator();

    // Generate a time-based UUID
    uuid_generate_time(uuid);


    // Convert the uuid_t to a string representation
    char uuid_str[37];
    uuid_unparse(uuid, uuid_str);
    std::cout << "UUID: " << gen(uuid_str) << std::endl;
    std::cout << "Time from UUID: " << TimestampFromUUID(gen(uuid_str)) << std::endl;
    // Output the generated UUID
    std::cout << "Time-based UUID: " << uuid_str << std::endl;

    return 0;
}
