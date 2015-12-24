#pragma once

#include <algorithm>
#include <ostream>
#include <functional>
#include <memory>
#include <type_traits>
#include <string>

class PCM_source;
class MediaItem;
class MediaItem_Take;
class MediaTrack;

template <typename T>
inline T bound_value(T lower, T n, T upper)
{
	return std::max(lower, std::min(n, upper));
}

template<typename T,typename U>
inline T map_value(U valin, U inmin, U inmax, T outmin, T outmax)
{
	static_assert(std::is_arithmetic<T>::value && std::is_arithmetic<U>::value, "Only arithmetic types supported");
	return outmin + ((outmax - outmin) * ((T)valin - (T)inmin)) / ((T)inmax - (T)inmin);
}

inline bool is_point_in_rect(int px, int py, int rx, int ry, int rw, int rh)
{
	return px>=rx && px<rx+rw && py>=ry && py<ry+rh;
}

inline bool is_alphaspacenumeric(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c < 'Z') || c == ' ';
}

template<typename T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	const std::size_t kMul = 0x9ddfea08eb382d69ULL;
	std::size_t a = (hasher(v) ^ seed) * kMul;
	a ^= (a >> 47);
	std::size_t b = (seed ^ a) * kMul;
	b ^= (b >> 47);
	seed = b * kMul;
}

// Get a value from a map style container without inserting an element if the key wasn't present
// and instead return default constructed value of the map value type
template<typename Key, typename Cont>
inline auto get_from_map(Cont& c, const Key& k)
{
	if (c.count(k) > 0)
		return c[k];
	return typename Cont::mapped_type();
}

std::string is_source_audio(PCM_source* src);

struct create_item_result
{
	MediaItem* item = nullptr;
	MediaItem_Take* take = nullptr;
	PCM_source* src = nullptr;
};

create_item_result create_item_with_take_and_source(MediaTrack* track, const char* fn);

class readbgbuf : public std::streambuf
{
public:
	int overflow(int c);
	std::streamsize xsputn(const char *buffer, std::streamsize n);
};

class readbg : public std::ostream
{
	readbgbuf buf;
public:
	readbg() :std::ostream(&buf) { }
};

class IValueConverter
{
public:
	virtual ~IValueConverter() {}
	virtual double fromNormalizedToValue(double x) = 0;
	virtual double toNormalizedFromValue(double x) = 0;
	virtual std::string toStringFromValue(double x) = 0;
	virtual double fromStringToValue(const std::string& x) = 0;
};

class LinearValueConverter : public IValueConverter
{
public:
	LinearValueConverter(double minval, double maxval) :
		m_min_value(minval), m_max_value(maxval) {}
	double fromNormalizedToValue(double x) override
	{ return map_value(x,0.0,1.0,m_min_value,m_max_value); }
	double toNormalizedFromValue(double x) override
	{ return map_value(x,m_min_value,m_max_value,0.0,1.0); }
	std::string toStringFromValue(double x) override
	{ return std::to_string(x); }
	double fromStringToValue(const std::string& x) override
	{
		return bound_value(m_min_value, atof(x.c_str()), m_max_value);
	}
private:
	double m_min_value = 0.0;
	double m_max_value = 1.0;
};

namespace MRP
{
	enum class Anchor
	{
		TopLeft, TopMiddle, TopRight,
		MiddleLeft, MiddleMiddle, MiddleRight,
		BottomLeft, BottomMiddle, BottomRight
	};

	template<typename T>
	class GenericRectangle
	{
	public:
		GenericRectangle() : m_x(T()), m_w(T()), m_y(T()), m_h(T()) {}
		GenericRectangle(T x, T y, T w, T h) :
			m_x(x), m_w(w), m_y(y), m_h(h) {}
		T getX() const noexcept { return m_x; }
		T getY() const noexcept { return m_y; }
		T getRight() const noexcept { return m_x + m_w; }
		T getBottom() const noexcept { return m_y + m_h; }
		T getMiddleX() const noexcept { return m_x + m_w / 2 }
		T getMiddleY() const noexcept { return m_y + m_h / 2 }
		T getWidth() const noexcept { return m_w; }
		T getHeight() const noexcept { return m_h; }

		void setX(T x) { m_x = x; }
		void setY(T y) { m_y = y;  }
		void setWidth(T w) { m_w = w;  }
		void setHeight(T h) { m_h = h;  }
		GenericRectangle<T> resized(T w, T h)
		{
			return GenericRectangle<T>(m_x, m_y, w, h);
		}
		GenericRectangle<T> moved(T x, T y)
		{
			return GenericRectangle<T>(x, y, m_w, m_h);
		}
		GenericRectangle centeredTo(T x, T y)
		{
			return GenericRectangle<T>(x - getWidth() / 2, y - getHeight() / 2, m_w, m_h);
		}
		GenericRectangle<T> leftShifted(T dx)
		{
			return GenericRectangle<T>(m_x + dx, y, m_w - dx, m_h);
		}
		GenericRectangle<T> rightShifted(T dx)
		{
			return GenericRectangle<T>(m_x, y, m_w + dx, m_h);
		}
		GenericRectangle<T> withHorizontalMargins(T margin)
		{
			return GenericRectangle<T>(m_x + margin, y, m_w - 2 * margin, m_h);
		}
		static GenericRectangle<T> anchoredToBottomOf(const GenericRectangle<T>& g,
			T x, T w, T h, T offset_from_bottom)
		{
			return GenericRectangle<T>(x,
				g.getBottom() - h - offset_from_bottom,
				w,
				h);
		}
		static GenericRectangle<T> anchoredTo(const GenericRectangle<T>& g, Anchor anchor, T w, T h)
		{
			if (anchor == Anchor::BottomLeft)
				return GenericRectangle<T>(g.m_x, g.getBottom() - h, w, h);
			if (anchor == Anchor::BottomRight)
				return GenericRectangle<T>(g.m_x + g.m_w - w, g.getBottom() - h, w, h);
			if (anchor == Anchor::Bottom)
				return GenericRectangle<T>(g.m_x, g.getBottom() - h, g.getWidth(), h);
			return GenericRectangle<T>();
		}
		static GenericRectangle<T> fromGridPositions(const GenericRectangle<T>& g,
			int griddivs, int x0, int y0, int x1, int y1)
		{
			T nx0 = (double)g.getWidth() / griddivs * x0;
			T ny0 = (double)g.getHeight() / griddivs * y0;
			T nx1 = (double)g.getWidth() / griddivs * x1;
			T ny1 = (double)g.getHeight() / griddivs * y1;
			T nw = nx1 - nx0;
			T nh = ny1 - ny0;
			return GenericRectangle<T>(nx0, ny0, nw, nh);
		}
		bool isValid() const noexcept 
		{ 
			return getX()<getRight() && getY()<getBottom(); 
		}
	private:
		T m_x;
		T m_w;
		T m_y;
		T m_h;
	};

	using Rectangle = GenericRectangle<int>;
}
class NoCopyNoMove
{
public:
	NoCopyNoMove(){}
	NoCopyNoMove(const NoCopyNoMove&) = delete;
	NoCopyNoMove& operator=(const NoCopyNoMove&) = delete;
	NoCopyNoMove(NoCopyNoMove&&) = delete;
	NoCopyNoMove& operator=(NoCopyNoMove&&) = delete;
};