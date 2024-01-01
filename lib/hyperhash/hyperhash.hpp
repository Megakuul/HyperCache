#ifndef HYPERHASH_H
#define HYPERHASH_H

#include <cstdint>
#include <cstring>
#include <string>

#include <byteswap.h>

#undef PERMUTE3
#define PERMUTE3(a, b, c) do { std::swap(a, b); std::swap(a, c); } while (0)


namespace hyperhash {
	// Magic numbers for 32-bit hashing.  Copied from Murmur3.
	inline static const uint32_t c1 = 0xcc9e2d51;
	inline static const uint32_t c2 = 0x1b873593;
	
	inline static uint32_t Rotate32(uint32_t val, int shift) {
		// Avoid shifting by 32: doing so yields an undefined result.
		return shift == 0 ? val : ((val >> shift) | (val << (32 - shift)));
	}

	inline static uint32_t Fetch32(const char *p) {
		uint32_t result;
		memcpy(&result, p, sizeof(result));
		return result;
	}

	// A 32-bit to 32-bit integer hash copied from Murmur3.
	inline static uint32_t fmix(uint32_t h) {
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;
		return h;
	}

  inline static uint32_t Mur(uint32_t a, uint32_t h) {
		// Helper from Murmur3 for combining two 32-bit values.
		a *= c1;
		a = Rotate32(a, 17);
		a *= c2;
		h ^= a;
		h = Rotate32(h, 19);
		return h * 5 + 0xe6546b64;
	}

	inline static uint32_t Hash32Len13to24(const char *s, size_t len) {
		uint32_t a = Fetch32(s - 4 + (len >> 1));
		uint32_t b = Fetch32(s + 4);
		uint32_t c = Fetch32(s + len - 8);
		uint32_t d = Fetch32(s + (len >> 1));
		uint32_t e = Fetch32(s);
		uint32_t f = Fetch32(s + len - 4);
		uint32_t h = static_cast<uint32_t>(len);

		return fmix(Mur(f, Mur(e, Mur(d, Mur(c, Mur(b, Mur(a, h)))))));
	}

	inline static uint32_t Hash32Len0to4(const char *s, size_t len) {
		uint32_t b = 0;
		uint32_t c = 9;
		for (size_t i = 0; i < len; i++) {
			signed char v = static_cast<signed char>(s[i]);
			b = b * c1 + static_cast<uint32_t>(v);
			c ^= b;
		}
		return fmix(Mur(b, Mur(static_cast<uint32_t>(len), c)));
	}

	inline static uint32_t Hash32Len5to12(const char *s, size_t len) {
		uint32_t a = static_cast<uint32_t>(len), b = a * 5, c = 9, d = b;
		a += Fetch32(s);
		b += Fetch32(s + len - 4);
		c += Fetch32(s + ((len >> 1) & 4));
		return fmix(Mur(c, Mur(b, Mur(a, d))));
	}
	
	inline uint32_t hash(std::string& key) {
		const char* s = key.c_str();
		const size_t len = key.length();
		
		if (len <= 24) {
    return len <= 12 ?
        (len <= 4 ? Hash32Len0to4(s, len) : Hash32Len5to12(s, len)) :
        Hash32Len13to24(s, len);
		}

		// len > 24
		uint32_t h = static_cast<uint32_t>(len), g = c1 * h, f = g;
		uint32_t a0 = Rotate32(Fetch32(s + len - 4) * c1, 17) * c2;
		uint32_t a1 = Rotate32(Fetch32(s + len - 8) * c1, 17) * c2;
		uint32_t a2 = Rotate32(Fetch32(s + len - 16) * c1, 17) * c2;
		uint32_t a3 = Rotate32(Fetch32(s + len - 12) * c1, 17) * c2;
		uint32_t a4 = Rotate32(Fetch32(s + len - 20) * c1, 17) * c2;
		h ^= a0;
		h = Rotate32(h, 19);
		h = h * 5 + 0xe6546b64;
		h ^= a2;
		h = Rotate32(h, 19);
		h = h * 5 + 0xe6546b64;
		g ^= a1;
		g = Rotate32(g, 19);
		g = g * 5 + 0xe6546b64;
		g ^= a3;
		g = Rotate32(g, 19);
		g = g * 5 + 0xe6546b64;
		f += a4;
		f = Rotate32(f, 19);
		f = f * 5 + 0xe6546b64;
		size_t iters = (len - 1) / 20;
		do {
			uint32_t a0 = Rotate32(Fetch32(s) * c1, 17) * c2;
			uint32_t a1 = Fetch32(s + 4);
			uint32_t a2 = Rotate32(Fetch32(s + 8) * c1, 17) * c2;
			uint32_t a3 = Rotate32(Fetch32(s + 12) * c1, 17) * c2;
			uint32_t a4 = Fetch32(s + 16);
			h ^= a0;
			h = Rotate32(h, 18);
			h = h * 5 + 0xe6546b64;
			f += a1;
			f = Rotate32(f, 19);
			f = f * c1;
			g += a2;
			g = Rotate32(g, 18);
			g = g * 5 + 0xe6546b64;
			h ^= a3 + a1;
			h = Rotate32(h, 19);
			h = h * 5 + 0xe6546b64;
			g ^= a4;
			g = bswap_32(g) * 5;
			h += a4 * 5;
			h = bswap_32(h);
			f += a0;
			PERMUTE3(f, h, g);
			s += 20;
		} while (--iters != 0);
		g = Rotate32(g, 11) * c1;
		g = Rotate32(g, 17) * c1;
		f = Rotate32(f, 11) * c1;
		f = Rotate32(f, 17) * c1;
		h = Rotate32(h + g, 19);
		h = h * 5 + 0xe6546b64;
		h = Rotate32(h, 17) * c1;
		h = Rotate32(h + f, 19);
		h = h * 5 + 0xe6546b64;
		h = Rotate32(h, 17) * c1;
		return h;
	};
}

#endif
