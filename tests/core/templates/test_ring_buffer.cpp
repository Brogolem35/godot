/**************************************************************************/
/*  test_ring_buffer.cpp                                                  */
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

#include "tests/test_macros.h"

TEST_FORCE_LINK(test_ring_buffer)

#include "core/templates/ring_buffer.h"

namespace TestRingBuffer {

TEST_CASE("[RingBuffer] Buffer initialization") {
	constexpr int RB_SIZE_POW = 4;
	RingBuffer<int> rb = RB_SIZE_POW;

	CHECK(rb.size() == (1 << RB_SIZE_POW));
	CHECK(rb.data_left() == 0);
	CHECK(rb.space_left() == rb.size() - 1);

	for (int i = 1; i < rb.size(); i++) {
		rb.write(i);

		CHECK(rb.size() == (1 << RB_SIZE_POW));
		CHECK(rb.data_left() == i);
		CHECK(rb.space_left() == (rb.size() - 1 - i));
	}

	for (int i = 1; rb.data_left() > 0; i++) {
		CHECK(rb.read() == i);
		CHECK(rb.data_left() == (rb.size() - 1 - i));
		CHECK(rb.space_left() == i);
	}
}

TEST_CASE("[RingBuffer] Buffer copy 1") {
	constexpr int RB_SIZE_POW = 4;
	RingBuffer<int> rb1 = RB_SIZE_POW;
	RingBuffer<int> rb2 = rb1;

	CHECK(rb1.size() == rb2.size());
	CHECK(rb1.data_left() == rb2.data_left());
	CHECK(rb1.space_left() == rb2.space_left());

	for (int i = 1; i < rb1.size(); i++) {
		rb1.write(i);
	}

	CHECK(rb1.data_left() != rb2.data_left());
	CHECK(rb1.space_left() != rb2.space_left());

	for (int i = 1; i < rb2.size(); i++) {
		// rb1 and rb2 have separate heap allocated buffers. These writes should not effect rb1.
		rb2.write(i + rb2.size());
	}

	CHECK(rb1.size() == rb2.size());
	CHECK(rb1.data_left() == rb2.data_left());
	CHECK(rb1.space_left() == rb2.space_left());
	while (rb1.data_left() > 0) {
		CHECK(rb1.size() == rb2.size());
		CHECK(rb1.data_left() == rb2.data_left());
		CHECK(rb1.space_left() == rb2.space_left());
		CHECK(rb1.read() != rb2.read());
	}
}

TEST_CASE("[RingBuffer] Buffer copy 2") {
	constexpr int RB_SIZE_POW = 4;
	RingBuffer<int> rb1 = RB_SIZE_POW;

	for (int i = 1; i < rb1.size(); i++) {
		rb1.write(i);
	}
	RingBuffer<int> rb2 = rb1;

	CHECK(rb1.size() == rb2.size());
	CHECK(rb1.data_left() == rb2.data_left());
	CHECK(rb1.space_left() == rb2.space_left());
	while (rb1.data_left() > 0) {
		CHECK(rb1.size() == rb2.size());
		CHECK(rb1.data_left() == rb2.data_left());
		CHECK(rb1.space_left() == rb2.space_left());
		CHECK(rb1.read() == rb2.read());
	}
}

TEST_CASE("[RingBuffer] Buffer copy 3") {
	constexpr int RB_SIZE_POW = 4;
	RingBuffer<int> rb1 = RB_SIZE_POW;

	for (int i = 1; i < rb1.size(); i++) {
		rb1.write(i);
	}
	RingBuffer<int> rb2 = rb1;

	rb2.read();
	CHECK(rb1.data_left() > rb2.data_left());
	CHECK(rb1.space_left() < rb2.space_left());
}

TEST_CASE("[RingBuffer] Read to buffer 1") {
	RingBuffer<int> rb1 = 2;
	int buf[3] = {7, 7, 7};

	for (int i = 0; i < 3; i++)
		rb1.write(i);

	int r = rb1.read(buf, 3);
	CHECK(r == 3);

	for (int i = 0; i < 3; i++) {
		CHECK(buf[i] == i);
	}
}

TEST_CASE("[RingBuffer] Read to buffer 2") {
	RingBuffer<int> rb1 = 2;
	int buf1[3] = {7, 7, 7};
	int buf2[3] = {7, 7, 7};

	for (int i = 0; i < 3; i++)
		rb1.write(i);

	int r1 = rb1.read(buf1, 3, false);
	CHECK(r1 == 3);
	CHECK(rb1.data_left() == 3);
	int r2 = rb1.read(buf2, 3);
	CHECK(r2 == 3);
	CHECK(rb1.data_left() == 0);

	for (int i = 0; i < 3; i++) {
		CHECK(buf1[i] == i);
		CHECK(buf1[i] == buf2[i]);
	}
}

TEST_CASE("[RingBuffer] Copy to buffer 1") {
	RingBuffer<int> rb1 = 2;
	int buf[3] = {7, 7, 7};

	for (int i = 0; i < 3; i++)
		rb1.write(i);

	int r = rb1.copy(buf, 0, 3);
	CHECK(r == 3);

	for (int i = 0; i < 3; i++) {
		CHECK(buf[i] == i);
	}
}

TEST_CASE("[RingBuffer] Copy to buffer 2") {
	RingBuffer<int> rb1 = 4;
	int buf[6] = {7, 7, 7, 7, 7, 7};

	for (int i = 0; i < 6; i++)
		rb1.write(i);

	int r = rb1.copy(buf, 4, 6);
	CHECK(r == 2);

	for (int i = 0; i < 2; i++) {
		CHECK(buf[i] == i + 4);
	}

	for (int i = 2; i < 6; i++) {
		CHECK(buf[i] == 7);
	}
}

} //namespace TestRingBuffer
