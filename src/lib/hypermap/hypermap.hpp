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

	/**
	 * HyperMap
	 *
	 * Open addressing hashmap that is optimized for read/write speed using quadratic probing.
	 * Tradeoff for this is that the map preallocates every slot with the largest type in the Derived_T types.
	 *
	 * Usage:
	 * HyperMap template requires a "Base_T" type as first template argument and a list of "Derived_T" types as second argument.
	 * The "Base_T" type must be the base class for the "Derived_T" types.
	 * System works like this: Every slot (bucket) can be set to one of the derived types, you can change the types in the slot
	 * at runtime by calling hypermap.set("key", ...) or by using the hypermap["key"] = ... overloader.
	 *
	 * The hypermap.get("key") function will return a "Base_T" pointer. This is then usually used through a virtual function that is overriden by the derived classes.
	 * For regularly manipulating elements (like e.g. updating values), use hypermap.get("key") and directly work with the element (use hypermap.set("key", ...) for type changing or initial allocation).
	 *
	 * Considerations:
	 * - The "mapsize" MUST be a power of two, this is required for correct hash-trimming. Initialization will throw an invalid_argument error if it's not.
	 * - HyperMap will preallocate "mapsize" buckets that are all sized like the largest type in "Derived_T".
	 * - The size of the map is immutable, the performance of the map will starve if the load is too high.
	 * - All "Derived_T" types must be statically upcastable to "Base_T".
	 * - All "Derived_T" types / their members must implement correct copy/move semantics (just so that the type can be deep copied and moved with "=")
	 *
	 */
	template <typename Base_T, typename... Derived_T>
	class HyperMap {
	public:
		HyperMap(uint16_t mapsize) {
			// Check if map is power of 2
			if (!(mapsize > 0 && (mapsize & (mapsize-1)) == 0))
				throw invalid_argument("Mapsize must be a power of two!");
			// Initialize map
			size = mapsize;
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
			if (this != &other) {
				try {
					// Copy constructor in this case uses default initialization and assignment instead of the copy constructor of the elements.
					// Technically it is feasible to use "operator new[] / delete[]"; This would allow direct construction of elements (e.g. direct copy construction).
					// But for performance reasons, the memory management of the HyperMap is handled with high-level "new[] and delete[]" functions (which can be quite more efficient then manual management).
					// HyperMap is primarly specified for fast access times and not for memory optimizations; it is usually not necessary to COPY the HyperMap.
					
					// Allocate block
					map = new HyperSlot<variant<Derived_T...>>[other.size];
					// Initialize every field with assignment overloader
					for (size_t i = 0; i < other.size; ++i) {
						map[i] = other.map[i];
					}
				} catch (...) {
					// Clean up resources on error
					size = 0;
					delete[] map;
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
		
		variant<Derived_T...>& operator[](const string& key) {
			HyperSlot<variant<Derived_T...>>* slot = probe(key, size, map);
			if (!slot) throw runtime_error("HyperMap exhausted; No free slot found!");

			if (slot->key=="") {
				// Update slot values (assignment operator must deallocate old resources if type is correctly implemented)
				slot->key = key;
				slot->val = variant<Derived_T...>();
			}

			return slot->val;
		};

		Base_T* get(const string& key) {
			HyperSlot<variant<Derived_T...>>* slot = probe(key, size, map);
			if (!slot) return nullptr;
			// Return BasePointer or nullptr if slot is empty
			return slot->key=="" ? nullptr : visit(BaseVisitor<Base_T>{}, slot->val);
		};

		Base_T* set(const string& key, const variant<Derived_T...>& val) {
			HyperSlot<variant<Derived_T...>>* slot = probe(key, size, map);
			if (!slot) throw runtime_error("HyperMap exhausted; No free slot found!");

			// Update slot values (assignment operator must deallocate old resources if type is correctly implemented)
			slot->key = key;
			slot->val = val;

			return visit(BaseVisitor<Base_T>{}, slot->val);
		};

		void del(const string& key) {
			HyperSlot<variant<Derived_T...>>* slot = probe(key, size, map);
			if (!slot) return;

			// Update slot values (assignment operator must deallocate old resources if type is correctly implemented)
			slot->key = "";
			slot->val = HyperSlot<variant<Derived_T...>>();
		};
		
	private:
		// Constants for quadratic probing
		inline static const uint8_t c1 = 1;
		inline static const uint8_t c2 = 3;

		// Function for probing / finding the requested key
		inline static HyperSlot<variant<Derived_T...>>* probe(const string& key, const uint16_t& size, const HyperSlot<variant<Derived_T...>>* map) {
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
				// Increment attempt (if every field was probed -> return)
				if (++att>size) return nullptr;
			}
		};
		
		uint16_t size;
		HyperSlot<variant<Derived_T...>>* map;
	};
}

#endif
