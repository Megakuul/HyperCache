#ifndef HYPERMAP_H
#define HYPERMAP_H

#include <cstdint>
#include <string>

using namespace std;

enum State {
	INACTIVE,
	ACTIVE,
};

template <typename T>
class HyperSlot {
	string key;
	T val;
	State state = INACTIVE;
};

template <typename T>
class HyperMap {
public:
	HyperMap(uint16_t map_size) {
		size = map_size;
		map = new T[size];
	};
	virtual ~HyperMap() {
		delete map;
	};
	HyperMap operator[](const string& key) {
		// idx = Hash(key) % size
		// if (map[idx].key == key)
		// return &map[idx].val;
		// else probe()
	};

	T* get(const string& key) {
		// idx = Hash(key) % size
		// if (map[idx].key == key)
		// return &map[idx].val;
		// else probe()
	};

	void set(const string& key, const T& val) {
		// idx = Hash(key) % size
		// if (map[idx].key == key)
		// &map[idx].val = val;
		// else probe()
	};
private:
	uint16_t size;
	HyperSlot<T>* map;
};

#endif
