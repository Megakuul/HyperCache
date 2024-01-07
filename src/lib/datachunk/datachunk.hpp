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
#include <limits>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <string>

using namespace std;

namespace datachunk {

	/**
	 * DataType of a DataChunk
	 */
	enum DataType {
		NONE = -1,
		PROTO = 0,
		COUNT = 1,
		GROUP = 2
	};
	
	class ProtoChunk;
	class CountChunk;
	class GroupChunk;

	/**
	 * DataChunk is a BaseClass for the different datatypes in HyperCache.
	 *
	 * The BaseClass implements virtual functions for all datatypes.
	 * To access a datatype, usually a BaseClass pointer is returned from HyperMap,
	 * this pointer can be used to directly access the datatype.
	 * If the datatype you want to access is not implemented, a runtime_error is thrown,
	 * this exception can directly be returned to the client
	 * (as it is the clients fault if the wrong datatype is accessed).
	 */
	class DataChunk {
	public:
		/**
		 * Get DataType of the implemented type
		 */
		virtual DataType get_type() noexcept { return NONE; };

		/**
		 * Get ProtoChunk data (for informations check ProtoChunk)
		 *
		 * Throws a runtime_error if the derived type is not ProtoChunk
		 */
		virtual pair<const uint8_t*, const uint8_t> get_proto() const {
			throw runtime_error("DataChunk is not of type PROTO");
		};
		/**
		 * Set ProtoChunk data (for informations check ProtoChunk)
		 *
		 * Throws a runtime_error if the derived type is not ProtoChunk
		 */
		virtual pair<const uint8_t*, const uint8_t> set_proto(uint8_t* new_bytes, uint8_t size) {
			throw runtime_error("DataChunk is not of type PROTO");
		};

		/**
		 * Get CountChunk data (for informations check CountChunk)
		 *
		 * Throws a runtime_error if the derived type is not CountChunk
		 */
		virtual uint64_t get_count() const {
			throw runtime_error("DataChunk is not of type COUNT");
		};
		/**
		 * Set CountChunk data (for informations check CountChunk)
		 *
		 * Throws a runtime_error if the derived type is not CountChunk
		 */
		virtual uint64_t set_count(uint64_t& new_count) {
			throw runtime_error("DataChunk is not of type COUNT");
		};
		/**
		 * Increment CountChunk data (for informations check CountChunk)
		 *
		 * Throws a runtime_error if the derived type is not CountChunk
		 */
		virtual uint64_t inc_count(int64_t& inc_count) {
			throw runtime_error("DataChunk is not of type COUNT");
		};

		/**
		 * Get GroupChunk data (for informations check GroupChunk)
		 *
		 * Throws a runtime_error if the derived type is not GroupChunk
		 */
		virtual unordered_set<weak_ptr<DataChunk>> get_group() const {
			throw runtime_error("DataChunk is not of type GROUP");
		};
		/**
		 * Push to GroupChunk data (for informations check GroupChunk)
		 *
		 * Throws a runtime_error if the derived type is not GroupChunk
		 */
		virtual unordered_set<weak_ptr<DataChunk>> push_group(DataChunk& chunk) {
			throw runtime_error("DataChunk is not of type GROUP");
		};
		/**
		 * Delete from GroupChunk data (for informations check GroupChunk)
		 *
		 * Throws a runtime_error if the derived type is not GroupChunk
		 */
		virtual unordered_set<weak_ptr<DataChunk>> del_group(DataChunk& chunk) {
			throw runtime_error("DataChunk is not of type GROUP");
		};

		/**
		 * Gets a set of groups that this datachunk is assigned to
		 */
		unordered_set<weak_ptr<GroupChunk>> get_assign() const {
			return group_assignments;
		};
		/**
		 * Adds a group assignment to this datachunk
		 */
		void add_assign(weak_ptr<GroupChunk> assign) {
			group_assignments.insert(assign);
		};
		/**
		 * Delets a group assignment to this datachunk
		 */
		void del_assign(weak_ptr<GroupChunk> assign) {
			group_assignments.erase(assign);
		};
	private:
		// TODO: Figure out a way for this problem
		/*
		  How is memory managed in HyperCache:
			Hyperslots return raw DataChunk pointers, this is not safe! This is why when doing something,
			hyperslots (that are btw. the only way to access datachunks) always return a LOCK asside, this
			lock is most likely like a mutex lock, and allows consistent data without smart_ptrs, as long as the
			hyperslot locks are locked, NOTHING will access / delete the memory, after the lock is released, the PTR shall not be used anymore.

			Now the problem is this: I can not persistently store ptrs to datachunks, this is dumb for the group
			type, there we have the option to just store raw "strings", but this would require on every read of the group
			bzw. on every query etc. to the group, we would need multiple hashes multiple hypermap accesses.
			It would be far more clean to have something like a ptr list and a group_assignment list on every
			datachunk, that can directly on destruction remove its assignment on the group and vise-verca.
			This unfortunaly dont work, the behaviour we need is what weak_ptrs provide
			(allows check if the ptr is still existent). For pretty logical reasons this doesnt work in my scenario
			(with inlined allocations).
			The problem essentially is that the HyperSlot determines the livetime of the DataChunk.
			
		 */
		// Group assignments is a set of weak_ptrs to all groups that the datachunk is assigned to
		unordered_set<weak_ptr<GroupChunk>> group_assignments;
	};

	/**
	 * ProtoChunk the default Datatype for HyperCache.
	 *
	 * The ProtoChunk is designed to contain raw bytes, usually this is a datastructure
	 * that has been serialized with Protobuffers on the client side.
	 *
	 * For fast read/writes of simple ProtoChunk values, this type implements two ways to store data.
	 *
	 * If the data fits into the quick_bytes slot (usually 255Bytes) it is directly inserted there.
	 * The quick_bytes slot is fixed allocated in the class metadata,
	 * this allows reading/writing with 0 runtime heap operations (making it very fast).
	 *
	 * If the data does not fit, it is automatically allocated in a regular vector object. 
	 */
	class ProtoChunk : public DataChunk {
	public:
		ProtoChunk(const ProtoChunk& other) = default;
		ProtoChunk(ProtoChunk&& other) = default;
		ProtoChunk& operator=(const ProtoChunk&) = default;
		ProtoChunk& operator=(ProtoChunk&&) = default;

		DataType get_type() noexcept override { return PROTO; };
	
		pair<const uint8_t*, const uint8_t> get_proto() const override {
			// If quick_mode is enabled, return quick bytes
			if (quick_mode) return make_pair(quick_bytes, quick_size);
			// If not, return the bytes from vector
			else return make_pair(bulk_bytes.data(), bulk_bytes.size());
		};
		
		pair<const uint8_t*, const uint8_t> set_proto(uint8_t* new_bytes, uint8_t size) override {
			if (size < quick_cap) {
				// Proto fits into the quick_bytes
				
				if (!quick_mode) {
					// If bulk was enabled before, deallocate bulk_bytes
					bulk_bytes = vector<uint8_t>();
					quick_mode = true;
				}
				// Update size
				quick_size = size;
				// Raw copy to the quick_bytes 
				memcpy(quick_bytes, new_bytes, quick_size);
				// Return byte ptr + size
				return make_pair(quick_bytes, quick_size);
			} else {
				// Proto does not fit into quick_bytes
				
				quick_mode = false;
				// Assign bytes to vector (basically reinitializes vector)
				bulk_bytes.assign(new_bytes, new_bytes + size);
				// Return byte ptr + size
				return make_pair(bulk_bytes.data(), bulk_bytes.size());
			}
		};
	private:
		// Capacity of quick field using uint8_t (255) bytes, with a 16K map, runtime overhead is at around 5MB
		// This seems much, but is a fine tradeoff regarding that most proto requests will fit into this.
		static constexpr uint_fast8_t quick_cap = numeric_limits<uint8_t>::max();
		// State of quick_mode
		bool quick_mode = true;
		// Size of the quick field
		uint8_t quick_size = 0;
		// quick_bytes array
		uint8_t quick_bytes[quick_cap];
		// bulk_bytes array
		vector<uint8_t> bulk_bytes;
	};

	/**
	 * CountChunk is a additional Datatype for HyperCache.
	 *
	 * This datatype is a simple counter that can be incremented / decremented.
	 *
	 * If the counter overflows (unsigned 64bit), expected modulo arithmetics come into force.
	 * (e.g. 2^64+1 = 1 || 0-1 = 2^64-1)
	 */
	class CountChunk : public DataChunk {
	public:
		CountChunk(const CountChunk& other) = default;
		CountChunk(CountChunk&& other) = default;
		CountChunk& operator=(const CountChunk&) = default;
		CountChunk& operator=(CountChunk&&) = default;
		
		DataType get_type() noexcept override { return COUNT; };
	
		uint64_t get_count() const override {
			return count;
		};
		uint64_t set_count(uint64_t& new_count) override {
			count = new_count;
			return count;
		};
		uint64_t inc_count(int64_t& inc_count) override {
			count += inc_count;
			return count;
		};
	private:
		uint64_t count = 0;
	};

	/**
	 * GroupChunk is a additional Datatype for HyperCache.
	 *
	 * This datatype is a group of datachunks.
	 */
	class GroupChunk : public DataChunk {
	public:
		GroupChunk(const GroupChunk& other) = default;
		GroupChunk(GroupChunk&& other) = default;
		GroupChunk& operator=(const GroupChunk&) = default;
		GroupChunk& operator=(GroupChunk&&) = default;
		
		DataType get_type() noexcept override { return GROUP; };
	
		unordered_set<weak_ptr<DataChunk>> get_group() const override {
			return group;
		};
		unordered_set<weak_ptr<DataChunk>> push_group(DataChunk& chunk) override {
			group.insert(chunk.get_weak());
			chunk.add_assign(this->get_weak());
			return group;
		};
		unordered_set<weak_ptr<DataChunk>> del_group(DataChunk* chunk) override {
			group.erase(chunk->get_weak());
			return group;
		};
	private:
		unordered_set<weak_ptr<DataChunk>> group;
	};

};

#endif
