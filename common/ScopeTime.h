#pragma once

#include <iostream>
#include <chrono>
#include <string>
class ScopeTime
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_prev;
public:
	ScopeTime() :m_begin(std::chrono::high_resolution_clock::now()), m_prev(m_begin)
	{

	}

	bool flew1sec()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto d = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_prev).count();
		if (d >= 1000)
		{
			m_prev = now;
			return true;
		}
		else
			return false;
	}

	int elapsedMilliseconds()
	{
		auto m_end = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(m_end - m_begin).count();
	}

	std::string elapsed()
	{
		std::string ret;
		auto m_end = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_end - m_begin).count();
		auto s = ms / 1000;
		if (s > 60)
		{
			auto m = s / 60;
			ret += std::to_string(m) + " min ";
		}
		s %= 60;
		ms %= 1000;
		ret += std::to_string(s) + "." + std::to_string(ms) + " s";
		return ret;
	}
};