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

#ifndef HYPERMAP_H
#define HYPERMAP_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <variant>

#include "hyperhash.hpp"

// Number of slots in the hashmap
#define MAP_SIZE 16384 // Must be a size that is powered by 2

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
			delete[] map;
		};
		HyperMap(HyperMap&& other) noexcept : size(other.size), map(other.map) {
			// Skip if same
			if (this != &other) {
				// Clear up resources on other
				other.size = 0;
				other.map = nullptr;
			};
		};
		HyperMap(const HyperMap& other) : size(other.size) {
			// TODO: Implement the full copy constructor (kinda struggling with teh allocation (new[] would init them, what is a unnecessary overhead, operator new[] on the other hand cannot be deallocated with the destructor (because they don't want to be deallocated with delete[] map, but with operator delete[](map);
			// Skip if same
			if (this != &other) {
				try {
				// Allocate block without initialization
				map = static_cast<HyperSlot<variant<Derived_T...>>*>(
					  operator new[](other.size * sizeof(HyperSlot<variant<Derived_T...>>))
			  );
				// Initialize every field with copy constructor
				for (size_t i = 0; i < other.size; ++i) {
					new (&map[i]) HyperSlot<variant<Derived_T...>>(other.map[i]);
				}
				} catch (...) {

					throw;
				}
			};
		};
		HyperMap& operator=(HyperMap&& other) noexcept {
			// Skip if same
			if (this != &other) {
				// Clear map before moving
				delete[] map;
				// Shallow copy
				size = other.size;
				map = other.map;
				// Clear up resources on other
				other.size = 0;
				other.map = nullptr;
			}
			return *this;
		};
		HyperMap& operator=(const HyperMap& other) {
			// Skip if same
			if (this != &other) {
				// Clean up old map
				delete[] map;
				// Update every field with copy semantics
				size = other.size;
				for (size_t i = 0; i < other.size; ++i) {
					map[i] = other.map[i];
				}
			};
			return *this;
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
				if (++att>size) throw runtime_error("HyperMap exhausted; No free slot found!");
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
				if (++att>size) throw runtime_error("HyperMap exhausted; No free slot found!");
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
