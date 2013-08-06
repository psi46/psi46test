// plot.cpp

#include "CImg.h"
using namespace cimg_library;

#include <string>
#include <vector>
#include <list>


// === data scope ===========================================================

class CColor
{
	unsigned char rgb[3];
public:
	CColor() {}
	CColor(unsigned char red, unsigned char green, unsigned char blue)
	{ rgb[0] = red; rgb[1] = green; rgb[2] = blue; }
	const unsigned char* Get() { return rgb; }
};


class CData
{
	double dt; // sample intervall [ns]
	double tmax; // time span [ns]
	CImg<double> data;

	void Draw(CImg<unsigned char> &img);
public:
	std::string name;
	CColor color;
	double offsetY;

	CData() : dt(0.0), tmax(0.0) {};
	void Read(const std::vector<double> &values);
	unsigned int Size() { return data.size(); }
	void Crop(double t1, double t2);
	void operator +=(const CData &data2);
	void operator +=(double a) { data += a; }
	void operator *=(double a) { data *= a; }
	double GetTSpan() { return tmax; }
	double GetTStep() { return dt; }
	unsigned int GetNPoints() { return data.size(); }
	CImg<double>& GetData() { return data; }
	void SetSignalName(std::string &signalName) { name = signalName; }
	void SetSignalName(const char *signalName) { name = signalName; }
	const std::string& GetSignalName() { return name; }
	void SetDrawingOptions(CColor col, double posY = 0.0) { color = col; offsetY = posY; }
	void DrawGraph(CImg<unsigned char> &img, double minV, double maxV);
};


class CDataList
{
	unsigned int n;
	CData *list[20];
public:
	CDataList() : n(0) {}
	~CDataList();
	void Add(CData *data) { if (n<20) list[n++] = data; else throw "Data list full"; }
	CData* Get(unsigned int index) { if (index<n) return list[index]; else throw "Plot does not exist"; }
	void Crop(double t1, double t2);
	void Draw(CImg<unsigned char> &img, double minV, double maxV);
	void Show(double minV, double maxV);
};



void CData::Read(const std::vector<double> &values)
{
	data.assign(values.data(), values.size());
	dt = 1.25; // ns
	tmax = dt*data.size();
}


void CData::Crop(double t1, double t2)
{
	int x0 = int(t1/dt);
	int x1 = int(t2/dt);
	int n  = data.size();

	if (x0 < 0)  x0 = 0;  if (x0 >= n) x0 = n-1;
	if (x1 < 0)  x1 = 0;  if (x1 >= n) x1 = n-1;

	if (x1 <= x0) return;
	data.crop(x0, x1);
	tmax = dt*data.size();
}


void CData::operator +=(const CData &data2)
{
	if (dt != data2.dt || tmax != data2.tmax) throw "cannot add different data";
	data += data2.data;	
}


void CData::DrawGraph(CImg<unsigned char> &img, double minV, double maxV)
{
	img.draw_graph(data, color.Get(), 1.0f, 1, 1, maxV - offsetY, minV - offsetY);
}



// === CDataList ============================================================

CDataList::~CDataList()
{
	for (unsigned int i=0; i<n; i++) delete list[i];
}


void CDataList::Crop(double t1, double t2)
{
	for (unsigned int i=0; i<n; i++) list[i]->Crop(t1, t2);
}


void CDataList::Draw(CImg<unsigned char> &img, double minV, double maxV)
{
	const double gridV =  100.0;
	const double gridT = 25.0; // ns

	const unsigned char col_black[3] = {   0,   0,   0 };
	const unsigned char col_gray[3]  = { 100, 180, 255 };

	img.fill(255);
	if (n == 0) return;

	double maxT = list[0]->GetTSpan();
	double gridX = img.width()/maxT; // pixel / ns
	double gridY = img.height()/(maxV-minV);

	// --- draw t-grid ---------------------------------------------------------
	for (double t = 0.0; t < maxT; t += gridT)
	{
		int x = int(t*gridX + 0.5);
		img.draw_line(x, 0, x, img.height()-20, col_gray, 1.0, 0x88888888 /* 0xCCCCCCCC */);
	}

	// --- draw v-grid ---------------------------------------------------------
	for (double v = minV+gridV; v < maxV; v += gridV)
	{
		int y = int(((maxV-v)*gridY) + 0.5);
		img.draw_line(0, y, img.width()-20, y, col_gray, 1.0, 0x88888888 /* 0xCCCCCCCC */);
	}

	// --- plots with zero line ------------------------------------------------
	for (unsigned i=0; i<n; i++)
	{
		int y = int((maxV - list[i]->offsetY)*img.height()/(maxV-minV) + 0.5);
		img.draw_line(0, y, img.width(), y, col_gray, 1.0, 0xffffffff /* 0xCCCCCCCC */);

		list[i]->DrawGraph(img, minV, maxV);
	}

	// --- draw t-axis ---------------------------------------------------------
	int axisY = img.height() - 20;
	img.draw_line(0, axisY, img.width(), axisY, col_black, 1.0);
	for (double t = 0.0; t < maxT; t += 2*gridT)
	{
		int x = int(t*gridX + 0.5);
		img.draw_line(x, axisY-2, x, axisY+2, col_black, 1.0);
		img.draw_text(x-10, axisY+4,  "%0.0f", col_black, 0, 1.0, 14, t);
	}

	// --- draw v-axis ---------------------------------------------------------
	for (double v = minV+gridV; v < maxV; v += gridV)
	{
		int y = int(((maxV-v)*gridY) + 0.5);
		img.draw_text(10, y-6, "%0.0f", col_black, 0, 1.0, 14, v);
	}

}


void CDataList::Show(double minV, double maxV)
{
	CImg<unsigned char> visu(1000,600, 1,3,0);
	Draw(visu, minV, maxV);
	CImgDisplay draw_disp(visu, "DTB Scope");
	while (!draw_disp.is_closed() && !draw_disp.is_keyESC())
	{
		draw_disp.wait();
		if (draw_disp.is_resized())
		{
			draw_disp.resize(false);
			visu.resize(draw_disp);
			Draw(visu, minV, maxV);
			visu.display(draw_disp);
		}
	}
}


// === end data scope =======================================================


void Scope(const char *title, std::vector<double> &values)
{
	const CColor col1 ( 200,  40,  20 );

	CDataList plot;
	
	CData *dat = new CData;
	try
	{
		dat->SetSignalName(title);
		dat->SetDrawingOptions(col1);
		dat->Read(values);
		plot.Add(dat);

		plot.Show(-500, 500);
	} catch (const char *) {};
}




void show_graph(CImg<double> &data, const char *const title=0,
                             const unsigned int plot_type=1, const unsigned int vertex_type=1,
                             const char *const labelx=0, const double xmin=0, const double xmax=0,
                             const char *const labely=0, const double ymin=0, const double ymax=0);


void show_graph(CImgDisplay &disp, CImg<double> &data,
	const unsigned int plot_type=1, const unsigned int vertex_type=1,
	const char *const labelx=0, const double xmin=0, const double xmax=0,
	const char *const labely=0, const double ymin=0, const double ymax=0);

void show_graph2(CImgDisplay &disp, CImg<double> &data,
	const unsigned int plot_type=1, const unsigned int vertex_type=1,
	const char *const labelx=0, const double xmin=0, const double xmax=0,
	const char *const labely=0, const double ymin=0, const double ymax=0);



void Scope2(const char *title, std::vector<double> &values)
{
	CImg<double> y(values.data(), values.size());

	float x0 =    0.0f;
	float x1 =  1.25f * values.size();
	const unsigned int plot_type = 1;
	const unsigned int vertex_type = 0;

	if (y.is_empty()) return;

	CImgDisplay disp;
	disp.assign(cimg_fitscreen(640,480,1),0,0).set_title(title);
	show_graph(disp, y, plot_type, vertex_type, "t / ns", x0, x1, "adc units");

//	y.display_graph(title ,plot_type, vertex_type, "t / ns", x0,x1, "adc units");
}


void PlotData(const char *title, const char *xaxis, const char *yaxis,
	double xmin, double xmax, std::vector<double> &values)
{
	CImg<double> y(values.data(), values.size());

	const unsigned int plot_type = 1;
	const unsigned int vertex_type = 0;

	if (y.is_empty()) return;

	CImgDisplay disp;
	disp.assign(cimg_fitscreen(640,480,1),0,0).set_title(title);
	show_graph(disp, y, plot_type, vertex_type, xaxis, xmin, xmax, yaxis);
}



//! Display 1d graph in an interactive window.
/**
	\param disp Display window.
	\param plot_type Plot type. Can be <tt>{ 0=points | 1=segments | 2=splines | 3=bars }</tt>.
	\param vertex_type Vertex type.
	\param labelx Title for the horizontal axis, as a C-string.
	\param xmin Minimum value along the X-axis.
	\param xmax Maximum value along the X-axis.
	\param labely Title for the vertical axis, as a C-string.
	\param ymin Minimum value along the X-axis.
	\param ymax Maximum value along the X-axis.
**/

void show_graph(CImgDisplay &disp, CImg<double> &data,
	const unsigned int plot_type, const unsigned int vertex_type,
	const char *const labelx, const double xmin, const double xmax,
	const char *const labely, const double ymin, const double ymax)
{
  if (data.is_empty()) return;

  if (!disp) disp.assign(cimg_fitscreen(640,480,1),0,0).set_title("CImg<%s>", data.pixel_type());

  const unsigned long siz = (unsigned long)data._width*data._height*data._depth, siz1 = cimg::max(1U,siz-1);
  const unsigned int old_normalization = disp.normalization();

  disp.show().flush()._normalization = 0;

  double y0 = ymin, y1 = ymax, nxmin = xmin, nxmax = xmax;
  if (nxmin==nxmax) { nxmin = 0; nxmax = siz1; }
  int x0 = 0, x1 = data.width()*data.height()*data.depth() - 1, key = 0;

  for (bool reset_view = true, resize_disp = false; !key && !disp.is_closed(); )
  {
    if (reset_view) { x0 = 0; x1 = data.width()*data.height()*data.depth()-1; y0 = ymin; y1 = ymax; reset_view = false; }

    CImg<double> zoom(x1-x0+1,1,1,data.spectrum());
    cimg_forC(data,c) zoom.get_shared_channel(c) = CImg<double>(data.data(x0,0,0,c),x1-x0+1,1,1,1,true);

    if (y0==y1) { y0 = zoom.min_max(y1); const double dy = y1 - y0; y0-=dy/20; y1+=dy/20; }
    if (y0==y1) { --y0; ++y1; }
    const CImg<int> selection = zoom.get_select_graph(disp,plot_type,vertex_type,
                                                       labelx,
                                                       nxmin + x0*(nxmax-nxmin)/siz1,
                                                       nxmin + x1*(nxmax-nxmin)/siz1,
                                                       labely,y0,y1);

    const int mouse_x = disp.mouse_x(), mouse_y = disp.mouse_y();
    if (selection[0]>=0)
	{
      if (selection[2]<0) reset_view = true;
      else
	  {
        x1 = x0 + selection[2]; x0+=selection[0];
        if (selection[1]>=0 && selection[3]>=0)
		{
          y0 = y1 - selection[3]*(y1-y0)/(disp.height()-32);
          y1-=selection[1]*(y1-y0)/(disp.height()-32);
        }
      }
    }
	else
	{
      bool go_in = false, go_out = false, go_left = false, go_right = false, go_up = false, go_down = false;

      switch (key = disp.key())
	  {
      case cimg::keyHOME : reset_view = resize_disp = true; key = 0; disp.set_key(); break;
      case cimg::keyPADADD : go_in = true; go_out = false; key = 0; disp.set_key(); break;
      case cimg::keyPADSUB : go_out = true; go_in = false; key = 0; disp.set_key(); break;
      case cimg::keyARROWLEFT : case cimg::keyPAD4 : go_left = true; go_right = false; key = 0; disp.set_key(); break;
      case cimg::keyARROWRIGHT : case cimg::keyPAD6 : go_right = true; go_left = false; key = 0; disp.set_key(); break;
      case cimg::keyARROWUP : case cimg::keyPAD8 : go_up = true; go_down = false; key = 0; disp.set_key(); break;
      case cimg::keyARROWDOWN : case cimg::keyPAD2 : go_down = true; go_up = false; key = 0; disp.set_key(); break;
      case cimg::keyPAD7 : go_left = true; go_up = true; key = 0; disp.set_key(); break;
      case cimg::keyPAD9 : go_right = true; go_up = true; key = 0; disp.set_key(); break;
      case cimg::keyPAD1 : go_left = true; go_down = true; key = 0; disp.set_key(); break;
      case cimg::keyPAD3 : go_right = true; go_down = true; key = 0; disp.set_key(); break;
      }

	  if (disp.wheel())
	  {
        if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) go_out = !(go_in = disp.wheel()>0);
        else if (disp.is_keySHIFTLEFT() || disp.is_keySHIFTRIGHT()) go_left = !(go_right = disp.wheel()>0);
        else go_up = !(go_down = disp.wheel()<0);
        key = 0;
      }

      if (go_in)
	  {
        const int
          xsiz = x1 - x0,
          mx = (mouse_x-16)*xsiz/(disp.width()-32),
          cx = x0 + (mx<0?0:(mx>=xsiz?xsiz:mx));
        if (x1-x0>4)
		{
          x0 = cx - 7*(cx-x0)/8; x1 = cx + 7*(x1-cx)/8;
          if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT())
		  {
            const double
              ysiz = y1 - y0,
              my = (mouse_y-16)*ysiz/(disp.height()-32),
              cy = y1 - (my<0?0:(my>=ysiz?ysiz:my));
              y0 = cy - 7*(cy-y0)/8; y1 = cy + 7*(y1-cy)/8;
          } else y0 = y1 = 0;
        }
      }

	  if (go_out)
	  {
        if (x0>0 || x1<(int)siz1)
		{
          const int delta_x = (x1-x0)/8, ndelta_x = delta_x?delta_x:(siz>1?1:0);
          const double ndelta_y = (y1-y0)/8;
          x0-=ndelta_x; x1+=ndelta_x;
          y0-=ndelta_y; y1+=ndelta_y;
          if (x0<0) { x1-=x0; x0 = 0; if (x1>=(int)siz) x1 = (int)siz1; }
          if (x1>=(int)siz) { x0-=(x1-siz1); x1 = (int)siz1; if (x0<0) x0 = 0; }
        }
      }

	  if (go_left)
	  {
        const int delta = (x1-x0)/5, ndelta = delta?delta:1;
        if (x0-ndelta>=0) { x0-=ndelta; x1-=ndelta; }
        else { x1-=x0; x0 = 0; }
        go_left = false;
      }

	  if (go_right)
	  {
        const int delta = (x1-x0)/5, ndelta = delta?delta:1;
        if (x1+ndelta<(int)siz) { x0+=ndelta; x1+=ndelta; }
        else { x0+=(siz1-x1); x1 = siz1; }
        go_right = false;
      }

	  if (go_up)
	  {
        const double delta = (y1-y0)/10, ndelta = delta?delta:1;
        y0+=ndelta; y1+=ndelta;
        go_up = false;
      }

      if (go_down)
	  {
        const double delta = (y1-y0)/10, ndelta = delta?delta:1;
        y0-=ndelta; y1-=ndelta;
        go_down = false;
      }
    }
  }
  disp._normalization = old_normalization;
}


void show_graph2(CImgDisplay &disp, CImg<double> &data,
	const unsigned int plot_type, const unsigned int vertex_type,
	const char *const labelx, const double xmin, const double xmax,
	const char *const labely, const double ymin, const double ymax)
{
	if (data.is_empty()) return;

	if (!disp) disp.assign(cimg_fitscreen(640,480,1),0,0).set_title("CImg<%s>", data.pixel_type());

	const unsigned long siz = (unsigned long)data._width*data._height*data._depth, siz1 = cimg::max(1U,siz-1);
	disp.normalization();

	disp.show().flush()._normalization = 0;

	double y0 = ymin, y1 = ymax, nxmin = xmin, nxmax = xmax;
	if (nxmin==nxmax) { nxmin = 0; nxmax = siz1; }
	int x0 = 0, x1 = data.width()*data.height()*data.depth() - 1;




	x0 = 0; x1 = data.width()*data.height()*data.depth()-1; y0 = ymin; y1 = ymax;

	CImg<double> zoom(x1-x0+1,1,1,data.spectrum());
	cimg_forC(data,c) zoom.get_shared_channel(c) = CImg<double>(data.data(x0,0,0,c),x1-x0+1,1,1,1,true);

	if (y0==y1) { y0 = zoom.min_max(y1); const double dy = y1 - y0; y0-=dy/20; y1+=dy/20; }
	if (y0==y1) { --y0; ++y1; }
	const CImg<int> selection = zoom.get_select_graph(disp,plot_type,vertex_type,
                                                       labelx,
                                                       nxmin + x0*(nxmax-nxmin)/siz1,
                                                       nxmin + x1*(nxmax-nxmin)/siz1,
                                                       labely,y0,y1);

	while (!disp.key() && !disp.is_closed())
	{
	    disp.wait();
	}

}
