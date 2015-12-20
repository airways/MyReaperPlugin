#pragma once

#include "lice_control.h"

struct fx_param_t
{
	int tracknum = -1;
	int fxnum = 0;
	int paramnum = 0;
};

struct point
{
	point() {}
	point(int x, int y) : m_x(x), m_y(y) {}
	int m_x = 0;
	int m_y = 0;
	fx_param_t m_x_target;
	fx_param_t m_y_target;
};

// Development test control
class TestControl : public LiceControl
{
public:
	TestControl(HWND parent, bool delwhendraggedoutside=false) : 
		LiceControl(parent), m_delete_point_when_dragged_outside(delwhendraggedoutside) {}
	void paint(LICE_IBitmap* bm) override;
	void mousePressed(const MouseEvent& ev) override;
	void mouseDoubleClicked(const MouseEvent& ev) override;
	void mouseMoved(const MouseEvent& ev) override;
	void mouseReleased(int x, int y) override;
	void mouseWheel(int x, int y, int delta) override;
	bool keyPressed(const ModifierKeys& modkeys, int keycode) override;
	std::function<void(int, double, double)> PointMovedCallback;
	fx_param_t* getFXParamTarget(int index, int which);
	std::string getType() const override { return "MultiXYControl"; }
private:
	
	std::vector<point> m_points;
	int find_hot_point(int x, int y);
	int m_hot_point = -1;
	float m_circlesize = 10.0f;
	bool m_mousedown = false;
	bool m_delete_point_when_dragged_outside = false;
	std::string m_test_text;
	void shift_points(double x, double y);
};

class WaveformControl : public LiceControl
{
public:
	WaveformControl(HWND parent);
	void setSource(PCM_source* src);
	void paint(LICE_IBitmap* bm) override;
	void mouseDoubleClicked(const MouseEvent& ev) override;
	bool keyPressed(const ModifierKeys& modkeys, int keycode) override;
	std::string getType() const override { return "WaveformControl"; }
private:
	std::shared_ptr<PCM_source> m_src;
	std::vector<double> m_minpeaks;
	std::vector<double> m_maxpeaks;
};


