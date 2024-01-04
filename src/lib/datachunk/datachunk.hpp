/**
 * HyperCache System
 *
 * Copyright (C) 2024  Linus Ilian Moser <linus.moser@megakuul.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATA_CHUNK_H
#define DATA_CHUNK_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

using namespace std;

class ProtoChunk;
class CountChunk;
class GroupChunk;

class DataChunk {
public:
	virtual int8_t get_type() { return -1; };
	
  virtual vector<uint8_t>* get_proto() { return nullptr; };
  virtual vector<uint8_t>* set_proto(vector<uint8_t>* new_bytes) { return nullptr; };
  
  virtual uint64_t* get_count() { return nullptr; };
  virtual uint64_t* set_count(uint64_t* new_count) { return nullptr; };
	virtual uint64_t* inc_count(int64_t* inc_count) { return nullptr; };

  virtual unordered_set<string>* get_group() { return nullptr; };
	virtual unordered_set<string>* set_group(unordered_set<string>* new_group) { return nullptr; };
  virtual unordered_set<string>* push_group(string* key) { return nullptr; };
	virtual unordered_set<string>* del_group(string* key) { return nullptr; };
	
private:
	unordered_set<weak_ptr<GroupChunk>> group_assignments; 
};

class ProtoChunk : public DataChunk {
public:
	int8_t get_type() override { return 0; };
	
  vector<uint8_t>* get_proto() override {
		return &bytes;
  };
  vector<uint8_t>* set_proto(vector<uint8_t>* new_bytes) override {
		bytes = *new_bytes;
		return &bytes;
  };
private:
  vector<uint8_t> bytes;
};
  
class CountChunk : public DataChunk {
public:
	int8_t get_type() override { return 1; };
	
  uint64_t* get_count() override {
		return &count;
  };
  uint64_t* set_count(uint64_t* new_count) override {
		count = *new_count;
		return &count;
  };
	uint64_t* inc_count(int64_t* inc_count) override {
		count += *inc_count;
		return &count;
	};
private:
  uint64_t count = 0;
};

class GroupChunk : public DataChunk {
public:
	int8_t get_type() override { return 2; };
	
  unordered_set<string>* get_group() override {
		return &group;
  };
  unordered_set<string>* set_group(unordered_set<string>* new_group) override {
		group = *new_group;
		return &group;
  };
	unordered_set<string>* push_group(string* key) override {
		group.insert(*key);
		return &group;
	};
	unordered_set<string>* del_group(string* key) override {
	  group.erase(*key);
		return &group;
	};
private:
  unordered_set<string> group;
};

#endif
