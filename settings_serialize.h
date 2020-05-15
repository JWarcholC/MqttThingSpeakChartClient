#ifndef SETTINGS_SERIALIZE_H
#define SETTINGS_SERIALIZE_H
#include <string>
#include <tuple>

namespace serialize{
template<typename T>
struct settings {
 T first;
 T second;
 T third;
};
}
#endif // SETTINGS_SERIALIZE_H
