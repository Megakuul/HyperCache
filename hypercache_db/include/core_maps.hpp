#ifndef CORE_MAPS_H
#define CORE_MAPS_H

#include <unordered_map>
#include <string>
#include <chrono>

#include "data_chunk.hpp"

#define CORE_MAP_SIZE 20000

using namespace std;

class CoreMap {
public:
  CoreMap() {
	map.reserve(CORE_MAP_SIZE);
  };

  /**
   * Get Slot
   */
  DataChunk* get_slot(string &key) {
	auto it = map.find(key);
	return it!=map.end() ? &map[key] : nullptr;
  };

  /**
   * Create Slot
   *
   * More efficient then set_slot if the key does not already exist
   */
  void create_slot(string& key, DataChunk& value) {
	if (!map.emplace(key, value).second)
	  map[key] = value;
  };

  /**
   * Set Slot
   *
   * More efficient then create_slot if the key may already exists
   */
  void set_slot(string& key, DataChunk& value) {
	map[key] = value;
  };

  /**
   * Delete Slot
   */
  void delete_slot(string& key) {
	map.erase(key);
  };
  

private:
  unordered_map<string, DataChunk> map;
};

class CoreTimeMap {
public:
  CoreTimeMap() {
	time_map.reserve(CORE_MAP_SIZE);
  };

  chrono::system_clock::time_point* get_slot(string &key) {
	auto it = time_map.find(key);
	return it!=time_map.end() ? &time_map[key] : nullptr;
  };
  
  void set_slot(string &key) {
	time_map[key] = chrono::system_clock::now();
  };
  
  void delete_slot(string &key) {
	time_map.erase(key);
  };
  
private:
  unordered_map<string, chrono::system_clock::time_point> time_map;
};

#endif
