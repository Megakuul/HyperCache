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

#include <chrono>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <variant>
#include <functional>
#include <atomic>

#include "hyperhash.hpp"

using namespace std;

namespace hypermap {
	/**
	 * Visitor to return the basepointer from the derived variant
	 */
	template <typename Base_T>
	struct BaseVisitor {
		template<typename T>
		Base_T* operator()(T& t) const {
			return static_cast<Base_T*>(&t);
		}
	};
	
	/**
	 * Key-Value datastructure for each slot
	 */
	template <typename T>
	class HyperSlot {
	public:
		/**
		 * Atom_Id indicating the state of the slot, if the slot is deleted / updated the id is incremented
		 * this will invalidate all SlotOperators that were bound to the state before.
		 *
		 * This is a atomic value.
		 */
		atomic<uint_fast16_t> atom_id = 0;
		/**
		 * Val_Lock is a shared_lock that is used for operations on the value (val) of this Slot.
		 *
		 * It is also a crucial part of the memory management strategy, as the val is only changed
		 * when this lock is fully (uniquely) locked.
		 */
		const shared_mutex val_lock;
		/**
		 * Meta_Lock is a shared_lock that is used for metadata (all others then val) operations at this Slot.
		 */
		const shared_mutex meta_lock;
		/**
		 * Key is the primary identifier of the slot
		 */
		string key = "";
		/**
		 * Val is the primary value of the slot
		 */
		T val;
		/**
		 * Time_Point is a timestamp of the last update to the slot
		 *
		 * TODO: I'm still not really sure how to implement this,
		 * I could just set the timestamp on Slot updates, but then it will be hard to
		 * reorganize data between Shards/Replicas.
		 * Setting the time on every write is very inefficient so I dont really know.
		 */
		chrono::system_clock::time_point time_point;
		/**
		 * Time_Duration is the duration until the slot is deleted
		 */
		chrono::system_clock::duration time_duration;
	};

	/**
	 * Operator that is returned for usage in higher level functions
	 *
	 * The operator implements a read and write function that can be used for operating with the datachunk.
	 *
	 * First argument of the read and write functions can be used to access the base_ptr to the datachunk.
	 *
	 * The DataChunk pointer may not be used outside the implemented and controlled functions, this is a crucial part of the memory management strategy.
	 */
	template <typename Base_T, typename Slot_T>
	class SlotOperator {
	public:
		SlotOperator<Base_T, Slot_T>(const HyperSlot<Slot_T>* slot) {
			slot_ptr = slot;
			operator_id = slot_ptr->atom_id;
		};
		/**
		 * Call read, to read the value from Slot
		 *
		 * The base_ptr to the datachunk can be accessed with the parameter
		 *
		 * Function will return false if the SlotOperator is invalid (Slot has been changed)
		 *
		 * IMPORTANT: DO NOT USE THE BASE_PTR OUTSIDE OF THIS CALLBACK
		 */
		bool read(function<void(const Base_T*)> callback) {
			// Only execute if slot_id == operator_id
			if (slot_ptr->atom_id!=operator_id) return false;
			
			const shared_lock<shared_mutex> lock(*slot_ptr->lock);
			// Ptr is safe, because when mutating, unique lock is enabled (which blocks the shared locks)
			const Base_T* data_ptr = visit(BaseVisitor<const Base_T>{}, slot_ptr->val);
			callback(data_ptr);
			return true;
		};
		/**
		 * Call write, to write to the value from Slot
		 *
		 * The base_ptr to the datachunk can be accessed with the parameter
		 *
		 * Function will return false if the SlotOperator is invalid (Slot has been changed)
		 *
		 * IMPORTANT: DO NOT USE THE BASE_PTR OUTSIDE OF THIS CALLBACK
		 */
		bool write(function<void(Base_T*)> callback) {
			// Only execute if slot_id == operator_id
			if (slot_ptr->id!=operator_id) return false;
			
			const unique_lock<shared_mutex> lock(*slot_ptr);
			// Ptr is safe, because unique lock is enabled
			Base_T* data_ptr = visit(BaseVisitor<Base_T>{}, slot_ptr->val);
			callback(data_ptr);
			return true;
		};
	private:
		const HyperSlot<Slot_T>* slot_ptr;
		const uint32_t operator_id;
	};

	/**
	 * HyperMap
	 *
	 * Open addressing hashmap that is optimized for read/write speed using quadratic probing.
	 * The map will preallocate every slot with the largest type in the Derived_T types.
	 *
	 *
	 * Usage:
	 *
	 * HyperMap template requires a "Base_T" type as first template argument and a list of "Derived_T" types as second argument.
	 * The "Base_T" type must be the base class for the "Derived_T" types.
	 * System works like this: Every HyperSlot (bucket) can be set to one of the derived types, you can change the types in the slot
	 * at runtime by calling hypermap.set("key", ...).
	 *
	 * The hypermap.get("key") function will return a SlotOperator. SlotOperators allow safe access to the HyperSlots value.
	 * If a HyperSlot value type is changed, the SlotOperator will automatically invalidate to ensure memory safety.
	 *
	 * SlotOperator operations are memory and threadsafe as long as the Map exists.
	 *
	 *
	 * Memory / Synchronisation Management:
	 *
	 * HyperMap uses a very dangerous memory and synchronisation strategy, that relies primarly on the fact that the Map and the Slots are immutable and exist over the lifetime of the app.
	 * Access to HyperSlot values is provided through raw pointers. To ensure memory safety, every operation with the HyperMap is done through SlotOperators.
	 * SlotOperator uses a shared_mutex to ensure memory safety and synchronisation at the same time. This may seem very dangerous and dumb,
	 * but is the consequence to the inline memory allocation.
	 *
	 * HyperSlots are allocated in a continuous block of memory, values are stored directly inlined to the HyperSlot block,
	 * this allows operations without any memory allocation call. The continuous memory block can also highly improve CPU cache hits.
	 *
	 * The HyperMap may not be destructed or moved while operations from other threads still access the old map; this is unsafe and will result in undefined behavior.
	 *
	 * HyperMap takes the dangerous memory strategy, in order to provide high performance.
	 *
	 *
	 * Considerations:
	 *
	 * - The "mapsize" MUST be a power of two, this is required for correct hash-trimming. Initialization will throw an invalid_argument error if it's not.
	 * - HyperMap will preallocate "mapsize" buckets that are all sized like the largest type in "Derived_T".
	 * - The size of the map is immutable, the performance of the map will starve if the load is too high.
	 * - Map synchronisation and memory management heavily relies on the fact that the map and the slots are immutable and exist over the lifetime of the app.
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
			occupied = 0;
			map = new HyperSlot<variant<Derived_T...>>[size];
		};
		virtual ~HyperMap() {
			delete[] map;
		};
		HyperMap(HyperMap&& other) noexcept : size(other.size), occupied(other.occupied), map(other.map) {
			// Skip if same
			if (this != &other) {
				// Clear up resources on other
				other.size = 0;
				other.occupied = 0;
				other.map = nullptr;
			};
		};
		HyperMap(const HyperMap& other) : size(other.size), occupied(other.occupied) {
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
					occupied = 0;
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
				occupied = other.occupied;
				size = other.size;
				map = other.map;
				// Clear up resources on other
				other.occupied = 0;
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
				occupied = other.occupied;
				size = other.size;
				for (size_t i = 0; i < other.size; ++i) {
					map[i] = other.map[i];
				}
			};
			return *this;
		};

		class HyperMapIterator {
		public:
			HyperMapIterator(uint16_t index, HyperMap& map) : idx(index), hypermap(map) {};

			HyperMapIterator& operator++() {
				// Skip all empty elements (key=="")
				do {
					idx++;
				} while (idx < hypermap.size && hypermap.map[idx].key=="");
				// Return Mapiterator
				return *this;
			};

			auto& operator*() const {
				// Return SlotOperator
				return SlotOperator<Base_T, variant<Derived_T...>>(&hypermap.map[idx]);
			};

			bool operator==(const HyperMapIterator& other) {
				return this->idx == other.idx;
			};
			
			bool operator!=(const HyperMapIterator& other) {
				return !(*this==other);
			};

			
		private:
			uint16_t idx;
			HyperMap& hypermap;
		};

		/**
		 * Returns the first iterator of the map
		 */
		HyperMapIterator begin() {
			// Skip all empty elements (key=="")
			uint16_t idx = 0;
			while (idx < size && map[idx].key=="") {
				idx++;
			}
			return HyperMapIterator(idx, *this);
		};

		/**
		 * Returns the last+1 iterator of the map
		 */
		HyperMapIterator end() {
			return HyperMapIterator(size, *this); 
		};

		/**
		 * Returns the occupied slots
		 */
		uint16_t load() {
			return occupied;
		}

		/**
		 * Gets a SlotOperator from Slot
		 *
		 * If the slot is not found it will return nullptr
		 */
		SlotOperator<Base_T, variant<Derived_T...>> get(const string& key) {
			return SlotOperator<Base_T, variant<Derived_T...>>(probe(key, size, map));
		};

		/**
		 * Overwrites a Slot value and returns a SlotOperator
		 *
		 * Throws a runtime error if no slot in the map is free.
		 */
		SlotOperator<Base_T, variant<Derived_T...>> set(const string& key, const variant<Derived_T...>& val) {
			HyperSlot<variant<Derived_T...>>* slot = probe(key, size, map);

			// Update slot values
			// (assignment operator must deallocate old resources if type is correctly implemented)
			{
				unique_lock<shared_mutex> lock(slot->lock);
				slot->key = key;
				slot->val = val;
			}
			occupied+=slot->key=="" ? 1 : 0;

			return SlotOperator<Base_T, variant<Derived_T...>>(slot);
		};

		/**
		 * Sets the slot to unoccupied and default initializes the value of the slot (by this it removes the old data)
		 */
		void del(const string& key) {
			HyperSlot<variant<Derived_T...>>* slot = probe(key, size, map);
			if (!slot) return;

			// Update slot values
			// (assignment operator must deallocate old resources if type is correctly implemented)
			{
				unique_lock<shared_mutex> lock(slot->lock);
				slot->key = "";
				slot->val = HyperSlot<variant<Derived_T...>>();
			}
			occupied--;
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
				idx = (hash + c1 * att + c2 * (att * att)) & (size-1);
				// Update slot
				slot = &map[idx];
				// Increment attempt (if every field was probed -> return)
				if (++att>size) return nullptr;
			}
		};
		
		uint16_t size;
		uint16_t occupied;
		HyperSlot<variant<Derived_T...>>* map;
	};
}

#endif
