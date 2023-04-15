#ifndef HELPER_H
#define HELPER_H

#include <nlohmann/json.hpp>
#include <visit_struct/visit_struct.hpp>
#include <iostream>
#include <fstream>
#include <type_traits>

template<typename>
struct is_std_vector : std::false_type {};

template<typename T, typename A>
  struct is_std_vector<std::vector<T,A>> : std::true_type {};

inline void write_json(nlohmann::json& data, const char* jsonPath)
{
  std::fstream of(jsonPath, std::fstream::out);
  if(!of.is_open())
  {
    return;
  }
  of << std::setw(4) << data << std::endl;
}

inline bool read_json(nlohmann::json& data, const char* jsonPath)
{
  std::ifstream jsonFile(jsonPath);
  if(jsonFile.is_open())
  {
    jsonFile >> data;
    return true;
  }
  return false;
}

template <typename T>
void deserialize(T& out, nlohmann::json& data)
{
  visit_struct::for_each(out,
                         [data](const char * name, auto & value)
                           {
                             if(data.contains(name))
                             {
                               value = data[name].get<typename std::remove_reference<decltype(value)>::type>();
                             }
                           });
}

template <typename T>
void deserialize(T& out, const char* jsonPath)
{
  using json = nlohmann::json;
  json data;
  read_json(data, jsonPath);
  deserialize(out, data);
}

template <typename T>
void serialize(const T& out, nlohmann::json& data)
{
  visit_struct::for_each(out,
                         [&data](const char * name, const auto & value)
                           {
                             if(visit_struct::traits::is_visitable<typename std::remove_reference<decltype(value)>::type>::value)
                             {
                               //doesnt work, something with type deduction
                               //serialize(value, data[name]);
                             }
                             else
                             {
                               data[name] = value;
                             }
                           });
}

template <typename T>
void serialize(const T& out, const char* jsonPath)
{
  using json = nlohmann::json;
  json data;
  serialize(out, data);
  write_json(data, jsonPath);
}

struct eq_visitor {
  bool result = true;

  template <typename T>
  void operator()(const char *, const T & t1, const T & t2) {
    result = result && (t1 == t2);
  }
};

template <typename T>
bool struct_eq(const T & t1, const T & t2) {
  eq_visitor vis;
  visit_struct::for_each(t1, t2, vis);
  return vis.result;
}

#endif