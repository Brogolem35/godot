/**************************************************************************/
/*  godot_linuxbsd.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "os_linuxbsd.h"

#include "core/templates/hash_set.h"
#include "core/templates/a_hash_set.h"

#include <iostream>
#include <ostream>

#if defined(SANITIZERS_ENABLED)
#include <sys/resource.h>
#endif

// For export templates, add a section; the exporter will patch it to enclose
// the data appended to the executable (bundled PCK).
#if !defined(TOOLS_ENABLED) && defined(__GNUC__)
static const char dummy[8] __attribute__((section("pck"), used)) = { 0 };

// Dummy function to prevent LTO from discarding "pck" section.
extern "C" const char *pck_section_dummy_call() __attribute__((used));
extern "C" const char *pck_section_dummy_call() {
	return &dummy[0];
}
#endif

int main(int argc, char *argv[]) {
#if defined(SANITIZERS_ENABLED)
	// Note: Set stack size to be at least 30 MB (vs 8 MB default) to avoid overflow, address sanitizer can increase stack usage up to 3 times.
	struct rlimit stack_lim = { 0x1E00000, 0x1E00000 };
	setrlimit(RLIMIT_STACK, &stack_lim);
#endif
	volatile size_t volatile_size_t = 0;

	for (const int size : { 8, 64, 1024, 4096, 20000 }) {
		{
			size_t time_ns = 0;
			size_t time_ns1 = 0;
			for (int run = 0; run < 20000000 / size; run++) {
				auto t0 = std::chrono::high_resolution_clock::now();
				AHashSet<Variant> set;
				for (int idx = 0; idx < size; idx ++) {
					// Test
					set.insert(idx);
				}
				auto t1 = std::chrono::high_resolution_clock::now();
				for (int idx = 0; idx < size; idx ++) {
					// Test
					set.insert(idx);
				}
				auto t2 = std::chrono::high_resolution_clock::now();

				time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
				time_ns1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
			}

			std::cout << "insert:" << size << std::endl;
			std::cout << time_ns / 1000 / 1000 << "ms\n";

			std::cout << "insert existing:" << size << std::endl;
			std::cout << time_ns1 / 1000 / 1000 << "ms\n";
		}
		{
			size_t time_ns = 0;
			for (int run = 0; run < 20000000 / size; run++) {
				AHashSet<Variant> set;
				for (int idx = 0; idx < size; idx ++) {
					// Test
					set.insert(idx);
				}
				auto t0 = std::chrono::high_resolution_clock::now();
				for (int idx = 0; idx < size; idx ++) {
					// Test
					set.erase(idx);
				}
				auto t1 = std::chrono::high_resolution_clock::now();
				time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
			}

			std::cout << "erase:" << size << std::endl;
			std::cout << time_ns / 1000 / 1000 << "ms\n";
		}
		{
			size_t time_ns = 0;
			for (int run = 0; run < 20000000 / size; run++) {
				AHashSet<Variant> set;
				for (int idx = 0; idx < size; idx ++) {
					// Test
					set.insert(idx);
				}
				size_t total = 0;
				auto t0 = std::chrono::high_resolution_clock::now();
				for (int idx = 0; idx < size; idx ++) {
					// Test
					total += set.has(idx);
				}
				auto t1 = std::chrono::high_resolution_clock::now();
				time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

				// Prevent compiling this out.
				volatile_size_t = total;
			}

			std::cout << "get:" << size << std::endl;
			std::cout << time_ns / 1000 / 1000 << "ms\n";
		}
	}
}
