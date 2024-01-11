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

#ifndef ATOMSTR_H
#define ATOMSTR_H

#include <cstdint>
#include <cstring>
#include <atomic>

using namespace std;

/**
 *
 */
class AtomStr {
public:
	AtomStr(const char* str) {
		_size = strlen(str);
		_data.store(new char[_size + 1]);
		memcpy(_data.load(), str, _size+1);
	};
	AtomStr(AtomStr&& other) noexcept {
		if (this != &other) {
			char* oldstr = other._data.load();
			other._data.store(nullptr);
			_data.store(oldstr);

			_size = other._size;
			other._size = 0;
		}
	};
	AtomStr(const AtomStr& other) {
		if (this != &other) {
			try {
				// Order doesn't matter, because it is a initializer
				_size = other._size;
				_data.store(new char[_size+1]);
				memcpy(_data.load(), other._data.load(), _size+1);
			} catch(...) {
				// Order doesn't matter, because it is a initializer
				delete[] _data.load();
				_data.store(nullptr);
				_size = 0;
				throw;
			}
		}
	};
	AtomStr& operator=(AtomStr&& other) noexcept {
		if (this != &other) {
			char* oldstr = _data.load();
			char* newstr = other._data.load();
			// Store before deletion is crucial, for threadsafe behavior
			other._data.store(nullptr);
			_data.store(newstr);
			delete[] oldstr;
			_size = other._size;
		  

			other._size = 0;
		}
		return *this;
	};
	AtomStr& operator=(const AtomStr& other) {
		if (this != &other) {
			try {
				char* newstr = new char[_size+1];
				// Cpy before storing is crucial, for threadsafe behavior
				memcpy(newstr, other._data.load(), _size+1);
				char* oldstr = _data.load();
				_data.store(newstr);
				delete[] oldstr;
				_size = other._size;
			} catch (...) {
				throw;
			}
		}
		return *this;
	};
	~AtomStr() {
		delete[] _data.load();
	};
	bool operator==(AtomStr& other) const {
		return !strcmp(this->_data.load(), other._data.load());
	};
	bool operator==(const char* other) const {
		return !strcmp(this->_data.load(), other);
	};

	// TODO: Problem, this is not thread safe, because it doesnt follow RCU as the rest of the op
	
	const char* c_str() const {
		const char* str = _data.load();
		return str==nullptr?"":str;
	};

	int_fast16_t size() const {
		return _size;
	};

private:
	atomic<char*> _data;
  int_fast16_t _size;
};

#endif
