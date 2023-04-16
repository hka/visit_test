#ifndef HELPER_H
#define HELPER_H

#include <nlohmann/json.hpp>
#include <visit_struct/visit_struct.hpp>
#include <iostream>
#include <fstream>

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
std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElementDeserialize(T& value, const std::string& name, const nlohmann::json& data)
{
  data.at(name).get_to(value);
}

template <typename T>
void deserialize(T& out, const nlohmann::json& data)
{
  visit_struct::for_each(out,
                         [data](const char * name, auto & value)
                           {
                             if(data.contains(name))
                             {
                               HandleStructElementDeserialize(value, name, data);
                             }
                           });
}

template <typename T>
std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElementDeserialize(T& value, const std::string& name, const nlohmann::json& data)
{
  deserialize(value, data[name]);
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
std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value>
 HandleStructElement(const T& e, const std::string& name, nlohmann::json& data)
{
  data[name] = e;
}

template <typename T>
void serialize(const T& out, nlohmann::json& data)
{
  visit_struct::for_each(out,
                         [&data](const char * name, const auto & value)
                           {
                             HandleStructElement(value, name, data);
                           });
}

template <typename T>
std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElement(const T& e, const std::string& name, nlohmann::json& data)
{
  serialize(e, data[name]);
}

template <typename T>
void serialize(const T& out, const char* jsonPath)
{
  using json = nlohmann::json;
  json data;
  serialize(out, data);
  write_json(data, jsonPath);
}

template <typename T>
bool struct_eq(const T & t1, const T & t2);

struct eq_visitor {
  bool result = true;

  template <typename T>
  std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value>
  operator()(const char *, const T & t1, const T & t2) {
    result = result && (t1 == t2);
  }
  template <typename T>
  std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
  operator()(const char *, const T & t1, const T & t2) {
    result = result && struct_eq(t1, t2);
  }
};

template <typename T>
bool struct_eq(const T & t1, const T & t2) {
  eq_visitor vis;
  visit_struct::for_each(t1, t2, vis);
  return vis.result;
}


#endif
