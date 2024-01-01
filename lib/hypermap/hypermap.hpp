#ifndef HYPERMAP_H
#define HYPERMAP_H

#include <cstdint>
#include <string>
#include <variant>

#include "hyperhash.hpp"

// Must be a size that is powered by 2
#define MAP_SIZE 16384

using namespace std;

namespace hypermap {
	enum State {
		UNINIT,
		EMTPY,
		ACTIVE,
	};

	template <typename T>
	class HyperSlot {
		string key;
		T val;
		State state = UNINIT;
	};

	template <typename Base_T>
	struct BaseVisitor {
		template<typename T>
		Base_T* operator()(T& t) const {
			return static_cast<Base_T*>(&t);
		}
	};

	template <typename Base_T, typename... Derived_T>
	class HyperMap {
	public:
		HyperMap() {
			size = MAP_SIZE;
			map = new variant<Derived_T...>[size];
		};
		virtual ~HyperMap() {
			delete map;
		};
		HyperMap operator[](const string& key) {
			// Benchmark, maybe % is better due to better distribution
			uint32_t idx = hyperhash::hash(key) & (size-1);
			// idx = Hash(key) % size
			// if (map[idx].key == key)
			// return &map[idx].val;
			// else probe()

			// Automatically initialize object / clear
		};

		Base_T* get(const string& key) {
			
			// idx = Hash(key) % size
			// if (map[idx].key == key)
			return visit(BaseVisitor<Base_T>{}, val);
			// return &map[idx].val;
			// else probe()

			// RETURN NULLPTR if empty or uninit
		};

		void set(const string& key, const variant<Derived_T...>& val) {
			uint32_t idx = hyperhash::hash(key) & (size-1);
			// if (map[idx].key == key)
			// &map[idx].val = val;
			// else probe()

			// Set no matter what
		};

		void del(const string& key) {

			// Just set flag to empty
		};
	private:
		uint16_t size;
		HyperSlot<variant<Derived_T...>>* map;
	};
}

#endif
