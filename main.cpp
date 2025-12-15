#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <iostream>
#include <chrono>

class CrosshairApp : public wxApp {
public:
    virtual bool OnInit();
};

class TestGLCanvas : public wxGLCanvas {
public:
    TestGLCanvas(wxWindow* parent, bool diagnostics, bool showTimer);

    void OnPaint(wxPaintEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnTimer(wxTimerEvent& event);
    void StartStressTest();

private:
    wxGLContext* m_context;
    wxPoint m_mousePos;
    wxTimer m_timer;
    bool m_init;
    
    // Diagnostics
    long long m_frameCount;
    std::chrono::steady_clock::time_point m_startTime;
    bool m_diagnostics;
    bool m_showTimer;
};

class CrosshairFrame : public wxFrame {
public:
    CrosshairFrame(const wxString& title, bool diagnostics, bool showTimer);
    TestGLCanvas* m_canvas;
};

wxIMPLEMENT_APP(CrosshairApp);

bool CrosshairApp::OnInit() {
    bool diagnostics = false;
    bool showTimer = false;
    for (int i = 1; i < argc; ++i) {
        wxString arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: ./crosshair_app [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --diagnostics  Enable stdout diagnostic logging" << std::endl;
            std::cout << "  --show-timer   Show elapsed time in window title" << std::endl;
            std::cout << "  --help, -h     Show this help message" << std::endl;
            return false;
        }

        if (arg == "--diagnostics") {
            diagnostics = true;
        } else if (arg == "--show-timer") {
            showTimer = true;
        }
    }

    CrosshairFrame *frame = new CrosshairFrame("Crosshair Tracker - OpenGL Stress Test", diagnostics, showTimer);
    frame->Show(true);
    frame->m_canvas->StartStressTest();
    return true;
}

CrosshairFrame::CrosshairFrame(const wxString& title, bool diagnostics, bool showTimer)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {
    m_canvas = new TestGLCanvas(this, diagnostics, showTimer);
}

TestGLCanvas::TestGLCanvas(wxWindow* parent, bool diagnostics, bool showTimer)
    : wxGLCanvas(parent, wxID_ANY, NULL, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
      m_init(false), m_frameCount(0), m_diagnostics(diagnostics), m_showTimer(showTimer) {
    m_context = new wxGLContext(this);
    m_startTime = std::chrono::steady_clock::now();
    
    Bind(wxEVT_PAINT, &TestGLCanvas::OnPaint, this);
    Bind(wxEVT_MOTION, &TestGLCanvas::OnMotion, this);
    m_timer.Bind(wxEVT_TIMER, &TestGLCanvas::OnTimer, this);
}

void TestGLCanvas::StartStressTest() {
    m_timer.Start(50);
}

void TestGLCanvas::OnPaint(wxPaintEvent& event) {
    // This is required even though we don't use it for drawing
    wxPaintDC dc(this);

    SetCurrent(*m_context);

    if (!m_init) {
        // Initialize OpenGL state if needed
        m_init = true;
        if (m_diagnostics) std::cout << "OpenGL Initialized" << std::endl;
    }
    
    m_frameCount++;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
    
    if (m_diagnostics) std::cout << "[Frame " << m_frameCount << " | " << elapsed << "ms] OnPaint start" << std::endl;

    if (m_showTimer) {
        if (wxFrame* frame = wxDynamicCast(GetParent(), wxFrame)) {
            frame->SetTitle(wxString::Format("Crosshair Tracker - %lld ms", elapsed));
        }
    }

    wxSize size = GetClientSize();
    glViewport(0, 0, size.x, size.y);

    // Clear background to white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw crosshair in black
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);

    // Map mouse coordinates to OpenGL coordinates (-1 to 1)
    // OpenGL origin is bottom-left, window origin is top-left
    float x = (float)m_mousePos.x / size.x * 2.0f - 1.0f;
    float y = -((float)m_mousePos.y / size.y * 2.0f - 1.0f);

    glBegin(GL_LINES);
    // Vertical line
    glVertex2f(x, -1.0f);
    glVertex2f(x, 1.0f);
    // Horizontal line
    glVertex2f(-1.0f, y);
    glVertex2f(1.0f, y);
    glEnd();

    glFlush();
    
    if (m_diagnostics) std::cout << "[Frame " << m_frameCount << "] Swapping buffers..." << std::endl;
    SwapBuffers();
    if (m_diagnostics) std::cout << "[Frame " << m_frameCount << "] Buffers swapped." << std::endl;
}

void TestGLCanvas::OnMotion(wxMouseEvent& event) {
    m_mousePos = event.GetPosition();
    // Refresh(); // Let the timer handle it
}

void TestGLCanvas::OnTimer(wxTimerEvent& event) {
    if (m_diagnostics) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
        std::cout << "[Timer | " << elapsed << "ms] Requesting Refresh/Update" << std::endl;
    }
    Refresh();
    Update();
}
