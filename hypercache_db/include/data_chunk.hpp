#ifndef DATA_CHUNK_H
#define DATA_CHUNK_H

#include <cstdint>
#include <vector>
#include <string>

using namespace std;

class DataChunk {
public:
  virtual vector<uint8_t>* get_proto() { return nullptr; };
  virtual vector<uint8_t>* set_proto(vector<uint8_t>*) { return nullptr; };
  
  virtual uint64_t* get_count() { return nullptr; };
  virtual uint64_t* set_count(uint64_t*) { return nullptr; };

  virtual vector<string>* get_iter() { return nullptr; };
  virtual vector<uint8_t>* set_iter(vector<string>*) { return nullptr; };
};

class ProtoChunk : public DataChunk {
public:
  vector<uint8_t>* get_proto() override {
	// TODO: Implement getter
	return nullptr;
  };
  vector<uint8_t>* set_proto(vector<uint8_t>*) override {
	// TODO: Implement setter
	return nullptr;
  };
private:
  vector<uint8_t> bytes;
};
  
class CountChunk : public DataChunk {
public:
  uint64_t* get_count() override {
	// TODO: Implement getter
	return nullptr;
  };
  uint64_t* set_count(uint64_t*) override {
	// TODO: Implement setter
	return nullptr;
  };
private:
  uint64_t count;
};

class IterChunk : public DataChunk {
public:
  vector<string>* get_iter() override {
	// TODO: Implement getter
	return nullptr;
  };
  vector<uint8_t>* set_iter(vector<string>*) override {
	// TODO: Implement setter
	return nullptr;
  };
private:
  vector<string> iter;
};

#endif
