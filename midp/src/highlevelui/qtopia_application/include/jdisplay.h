#ifndef _JDISPLAY_H_
#define _JDISPLAY_H_

#include <QStackedWidget>
#include "japplication.h"

class QPixmap;

/*!
  \brief J2ME diplay manager sigleton

  Being the main widget of the JVM application, JDisplay manages window states and stores
  application-global backbuffer for either JCanvas lfpport displayable or lfjport screen.
  Implementation-specific screens should be added to this widget as to QStackedWidget.
*/

class JDisplay: public QStackedWidget
{
  Q_OBJECT
  protected:
    JDisplay();
    virtual ~JDisplay();
  public:
    inline static JDisplay *current()
    {
      if (!m_instance)
        init();
      return m_instance;
    }

    static void init();
    static void destroy();

    void setFullScreenMode(bool mode); // set display maximized/regular state
    inline bool fullScreenMode() const { return m_fullscreen; }

    inline void setReversedOrientation(bool reverse) { Q_UNUSED(reverse); } // Set display rotation
    inline bool reversedOrientation() const { return m_reversed; }

    // Display sizes cannot be calculated here, thus they are calculated by a Canvas native peer and stored here for convenience
    inline int displayWidth() const { return m_width; }
    inline int displayHeight() const { return m_height; }
    void setDisplayWidth(int w);
    void setDisplayHeight(int h);
    
    inline int dpi() const { return m_dpi; }

  protected:
    void resizeEvent(QResizeEvent *e);
    bool event(QEvent *);
  private:
    void resizeBackBuffer(int newWidth, int newHeight); // Handle backbuffer resizing
    static JDisplay *m_instance;

    bool m_fullscreen;
    bool m_reversed;

    int m_width;
    int m_height;
    
    int m_dpi;

    RunConfig *cfg;
};

#endif // _JDISPLAY_H_
