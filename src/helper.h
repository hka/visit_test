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

// ===================================================================
// Deserialize

template <typename T>
void deserialize(T& out, const nlohmann::json& data);

template <typename T>
std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value && !is_std_vector<std::decay_t<T>>::value>
HandleStructElementDeserialize(T& value, const std::string& name, const nlohmann::json& data)
{
  data.at(name).get_to(value);
}

template <typename T>
std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElementDeserialize(T& value, const std::string& name, const nlohmann::json& data)
{
  deserialize(value, data[name]);
}

template <typename T>
std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElementDeserializeIx(T& value, const std::string& name, size_t ix, const nlohmann::json& data)
{
  data.at(name).at(ix).get_to(value);
}

template <typename T>
std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElementDeserializeIx(T& value, const std::string& name, size_t ix, const nlohmann::json& data)
{
  deserialize(value, data[name][ix]);
}

template <typename T>
void
HandleStructElementDeserialize(std::vector<T>& value, const std::string& name, const nlohmann::json& data)
{
  value.resize(data[name].size());
  for(size_t ii = 0; ii < value.size(); ++ii)
  {
    HandleStructElementDeserializeIx(value[ii], name, ii, data);
  }
}

// -------------------------------------------------------------------

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
void deserialize(T& out, const char* jsonPath)
{
  using json = nlohmann::json;
  json data;
  read_json(data, jsonPath);
  deserialize(out, data);
}

// ===================================================================
// Serialize

template <typename T>
void serialize(const T& out, nlohmann::json& data);

template <typename T>
std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value && !is_std_vector<std::decay_t<T>>::value>
 HandleStructElement(const T& e, const std::string& name, nlohmann::json& data)
{
  data[name] = e;
}

template <typename T>
std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElement(const T& e, const std::string& name, nlohmann::json& data)
{
  serialize(e, data[name]);
}

template <typename T>
std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElementIx(const T& e, const std::string& name, size_t ix, nlohmann::json& data)
{
  data[name][ix] = e;
}
template <typename T>
std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
HandleStructElementIx(const T& e, const std::string& name, size_t ix, nlohmann::json& data)
{
  serialize(e, data[name][ix]);
}

template <typename T>
void
HandleStructElement(const std::vector<T>& e, const std::string& name, nlohmann::json& data)
{
  for(size_t ii = 0; ii < e.size(); ++ii)
  {
    HandleStructElementIx(e[ii], name, ii, data);
  }
}

// -------------------------------------------------------------------

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
void serialize(const T& out, const char* jsonPath)
{
  using json = nlohmann::json;
  json data;
  serialize(out, data);
  write_json(data, jsonPath);
}

// ===================================================================
// Compare

template <typename T>
bool struct_eq(const T & t1, const T & t2);

struct eq_visitor {
  bool result = true;

  template <typename T>
  std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value && !is_std_vector<std::decay_t<T>>::value>
  operator()(const char *, const T & t1, const T & t2) {
    result = result && (t1 == t2);
  }
  template <typename T>
  std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
  operator()(const char *, const T & t1, const T & t2) {
    result = result && struct_eq(t1, t2);
  }

  template <typename T>
  std::enable_if_t<!visit_struct::traits::is_visitable<std::decay_t<T>>::value>
  cmp(const T & t1, const T & t2) {
    result = result && (t1 == t2);
  }
  template <typename T>
  std::enable_if_t<visit_struct::traits::is_visitable<std::decay_t<T>>::value>
  cmp(const T & t1, const T & t2) {
    result = result && struct_eq(t1, t2);
  }

  template <typename T>
  void
  operator()(const char *, const std::vector<T> & t1, const std::vector<T> & t2) {
    if(t1.size() != t2.size())
    {
      result = result && false;
    }
    else
    {
      for(size_t ii = 0; ii < t1.size(); ++ii)
      {
        //result = result && cmp(t1, t2);
        cmp(t1[ii], t2[ii]);
      }
    }
  }
};

template <typename T>
bool struct_eq(const T & t1, const T & t2) {
  eq_visitor vis;
  visit_struct::for_each(t1, t2, vis);
  return vis.result;
}


#endif
