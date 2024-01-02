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
	template <typename T>
	class HyperSlot {
	public:
		string key = "";
		T val;
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
			map = new HyperSlot<variant<Derived_T...>>[size];
		};
		virtual ~HyperMap() {
			delete map;
		};
		// Constants for quadratic probing
		inline static const uint8_t c1 = 1;
		inline static const uint8_t c2 = 3;
		
		variant<Derived_T...>& operator[](const string& key) {
			// Calculate initial hash
			uint32_t hash = hyperhash::hash(key);
			// Trimm down hash to index
			uint32_t idx = hash & (size-1);
			// Set attempt to 0
			uint32_t att = 0;

			// Initial slot
			HyperSlot<variant<Derived_T...>>* slot = &map[idx];

			// Probe until slot is uninitialized || the correct slot
			while (slot->key!=key && slot->key!="") {
				// Quadratic probing function
				idx = (hash + c1 * att + c2 * (att * att)) % size;
				// Update slot
				slot = &map[idx];
				// Increment attempt (if every field was probed -> nullptr)
				if (++att>size) return nullptr;
			}

			if (slot->key=="") {
				slot->key = key;
				slot->val = variant<Derived_T...>();
			}

			return slot->val;
		};

		Base_T* get(const string& key) {
			// Calculate initial hash
			uint32_t hash = hyperhash::hash(key);
			// Trimm down hash to index
			uint32_t idx = hash & (size-1);
			// Set attempt to 0
			uint32_t att = 0;

			// Initial slot
			HyperSlot<variant<Derived_T...>>* slot = &map[idx];

			// Probe until slot is uninitialized || the correct slot
			while (slot->key!=key && slot->key!="") {
				// Quadratic probing function
				idx = (hash + c1 * att + c2 * (att * att)) % size;
				// Update slot
				slot = &map[idx];
				// Increment attempt (if every field was probed -> nullptr)
				if (++att>size) return nullptr;
			}
			// Return BasePointer or nullptr if slot is empty
			return slot->key=="" ? nullptr : visit(BaseVisitor<Base_T>{}, slot->val);
		};

		Base_T* set(const string& key, const variant<Derived_T...>& val) {
			// Calculate initial hash
			uint32_t hash = hyperhash::hash(key);
			// Trimm down hash to index
			uint32_t idx = hash & (size-1);
			// Set attempt to 0
			uint32_t att = 0;

			// Initial slot
			HyperSlot<variant<Derived_T...>>* slot = &map[idx];

			// Probe until slot is uninitialized || the correct slot
			while (slot->key!="" && slot->key!=key) {
				// Quadratic probing function
				idx = (hash + c1 * att + c2 * (att * att)) % size;
				// Update slot
				slot = &map[idx];
				// Increment attempt (if every field was probed -> nullptr)
				if (++att>size) return nullptr;
			}

			// Update slot values
			slot->key = key;
			slot->val = val;

			return visit(BaseVisitor<Base_T>{}, slot->val);
		};

		void del(const string& key) {
			// Calculate initial hash
			uint32_t hash = hyperhash::hash(key);
			// Trimm down hash to index
			uint32_t idx = hash & (size-1);
			// Set attempt to 0
			uint32_t att = 0;

			// Initial slot
			HyperSlot<variant<Derived_T...>>* slot = &map[idx];

			// Probe until slot is uninitialized || the correct slot
			while (slot->key!="" && slot->key!=key) {
				// Quadratic probing function
				idx = (hash + c1 * att + c2 * (att * att)) % size;
				// Update slot
				slot = &map[idx];
				// Increment attempt (if every field was probed -> nullptr)
				if (++att>size) return;
			}

			slot->key = "";
			slot->val = HyperSlot<variant<Derived_T...>>();
		};
	private:
		uint16_t size;
		HyperSlot<variant<Derived_T...>>* map;
	};
}

#endif
