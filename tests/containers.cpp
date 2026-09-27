/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Containers, strings, algorithms and iterators
#include "test.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <deque>
#include <forward_list>
#include <iterator>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

int main()
{
	std::vector<int> v{5, 3, 1, 4, 2};
	v.push_back(6);
	v.insert(v.begin(), 0);
	std::sort(v.begin(), v.end());
	TEST_ASSERT((v == std::vector<int>{0, 1, 2, 3, 4, 5, 6}));
	std::reverse(v.begin(), v.end());
	TEST_ASSERT(std::distance(v.begin(), std::find(v.begin(), v.end(), 4)) == 2);
	TEST_ASSERT(v.at(1) == 5);
	TEST_ASSERT(std::accumulate(v.begin(), v.end(), 0) == 21);
	TEST_ASSERT(std::count_if(v.begin(), v.end(), [](int x) { return x & 1; }) == 3);
	v.erase(std::remove_if(v.begin(), v.end(), [](int x) { return x > 3; }), v.end());
	TEST_ASSERT(v.size() == 4);

	std::vector<bool> vb(20);
	vb[3] = true;
	TEST_ASSERT(vb.size() == 20 and vb[3] and not vb[4]);

	std::array<uint8_t, 8> a{};
	std::iota(a.begin(), a.end(), 1);
	TEST_ASSERT(*std::max_element(a.begin(), a.end()) == 8);
	TEST_ASSERT(std::binary_search(a.begin(), a.end(), 5));

	std::deque<int> d{1, 2};
	d.push_front(0);
	d.push_back(3);
	TEST_ASSERT(d.front() == 0 and d.back() == 3 and d.size() == 4);

	std::list<int> l{3, 1, 2};
	l.sort();
	l.push_front(0);
	TEST_ASSERT(l.front() == 0 and l.back() == 3 and l.size() == 4);
	std::forward_list<int> fl{1, 2, 3};
	fl.reverse();
	TEST_ASSERT(fl.front() == 3);

	std::map<int, int> m{{1, 10}, {2, 20}};
	m[3] = 30;
	m.erase(1);
	TEST_ASSERT(m.size() == 2 and m.lower_bound(2)->second == 20 and m.rbegin()->first == 3);
	std::multimap<int, int> mm{{1, 1}, {1, 2}};
	TEST_ASSERT(mm.count(1) == 2);
	std::set<int> s{3, 1, 2};
	TEST_ASSERT(*s.begin() == 1 and s.count(4) == 0);

	std::unordered_map<int, int> um{{1, 2}};
	um[5] = 6;
	um.reserve(50);
	TEST_ASSERT(um.at(5) == 6 and um.size() == 2 and um.bucket_count() >= 50);
	std::unordered_set<uint16_t> us{1, 2, 3};
	us.erase(2);
	TEST_ASSERT(us.count(3) == 1 and us.count(2) == 0);

	std::priority_queue<int> pq;
	for (int x : {4, 9, 2}) pq.push(x);
	std::stack<int> st;
	st.push(pq.top());
	std::queue<int> q;
	q.push(st.top());
	TEST_ASSERT(q.front() == 9);

	std::bitset<40> b;
	b.set(3).set(39);
	TEST_ASSERT(b.count() == 2 and b.test(39) and b.to_ullong() == ((1ull << 39) | 8));

	std::string str = "hello";
	str += " world";
	str.replace(0, 1, "H");
	std::string_view sv{str};
	TEST_ASSERT(str.size() == 11 and sv.find("world") == 6 and sv.substr(0, 5) == "Hello");
	TEST_ASSERT(std::to_string(-42) == "-42");
	str.resize(40, '!');
	TEST_ASSERT(str.back() == '!' and str.find('!') == 11);

	test::finish();
}
