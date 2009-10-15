#include <QPixmap>
#include <QResizeEvent>
#include <QScreen>
#include <QDesktopWidget>

#include <midp_logging.h>

#include "jdisplay.h"
#include "jmutableimage.h"

JDisplay *JDisplay::m_instance = NULL;

JDisplay::JDisplay()
  : QStackedWidget(NULL), m_fullscreen(false), m_reversed(false), m_width(-1), m_height(-1)
{
  cfg = &(JApplication::instance()->cfg);

  setWindowTitle("phoneME");
  if (cfg->sFixed)
  {
    m_width = cfg->sWidth;
    m_height = cfg->sHeight;
  }
  else
  {
    QSize screenSize = JApplication::desktop()->availableGeometry().size();
    m_width = screenSize.width();
    m_height = screenSize.height();
  }
  
  //QScreen *screen = QScreen::instance();
  m_dpi = JApplication::desktop()->logicalDpiY();
  if (cfg->sFixed)
  {
    m_dpi = (int)(m_dpi * cfg->k);
  }

  setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

  if (cfg->sForceFullscreen)
  {
    setFullScreenMode(true);
  }
}

JDisplay::~JDisplay()
{
}

void JDisplay::init()
{
  if (!m_instance)
    m_instance = new JDisplay;
}

void JDisplay::destroy()
{
  if (m_instance)
  {
    delete m_instance;
    m_instance = NULL;
  }
}

void JDisplay::resizeEvent(QResizeEvent *e)
{
  resizeBackBuffer(e->size().width(), e->size().height());
  if (cfg->sFixed)
  {
    if (m_width<0)
      m_width = cfg->sWidth;
    if (m_height<0)
      m_height = cfg->sHeight;
  }
  else
  {
    if (m_width<0)
      m_width = width();
    if (m_height<0)
      m_height = height();
  }
}

// resize backbuffer only if required size is bigger than qpixmap size to minimize amount of pixmap reallocations
void JDisplay::resizeBackBuffer(int newWidth, int newHeight)
{
  JMutableImage *backBuffer = JMutableImage::fromHandle(NULL);
  if (cfg->sFixed)
  {
    if (backBuffer->isNull())
      *backBuffer = JMutableImage(cfg->sWidth, cfg->sHeight);
  }
  else
  {
    if ((backBuffer->isNull()) || (newWidth>backBuffer->width() || newHeight>backBuffer->height()))
    {
      newWidth = qMax(backBuffer->width(), newWidth);
      newHeight = qMax(backBuffer->height(), newHeight);
      *backBuffer = JMutableImage(newWidth, newHeight);
      //backBuffer->fill(0);
    }
  }
}

void JDisplay::setFullScreenMode(bool mode)
{
  if (cfg->sForceFullscreen) mode=true;

  if (mode!=m_fullscreen) // Do we actually need to change state?
  {
    m_fullscreen = mode;
    if (mode)
    {
      qDebug("JDisplay: fullscreen ON\n");
      QString title = windowTitle();
      if (mode)
        setWindowTitle( QLatin1String("_allow_on_top_"));
      setWindowState(windowState() ^ Qt::WindowFullScreen);
      setWindowTitle(title);
    }
    else
    {
      qDebug("JDisplay: fullscreen OFF\n");
      setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
  }
  if (!currentWidget() || !currentWidget()->inherits("JForm"))
  {
    setDisplayWidth(width());
    setDisplayHeight(height());
  }
}

void JDisplay::setDisplayWidth(int w)
{
  if (cfg->sFixed)
    m_width = cfg->sWidth;
  else
    m_width = w;
}

void JDisplay::setDisplayHeight(int h)
{
  if (cfg->sFixed)
    m_height = cfg->sHeight;
  else
    m_height = h;
}

bool JDisplay::event(QEvent *event)
{
    if(event->type() == QEvent::WindowDeactivate)
    {
        lower();        
        if (cfg->gQuitOnHide)
        {
          qDebug() << "quitOnHide: ON";
          JApplication::instance()->quit();
        }
        else
        {
          if (cfg->sQvga) JApplication::instance()->QvgaStop(); //
        }
    }
    else if(event->type() == QEvent::WindowActivate)
    {
        QString title = windowTitle();
        if (m_fullscreen)
          setWindowTitle(QLatin1String("_allow_on_top_"));
        raise();
        setWindowTitle(title);
        this->update(); //
        if (cfg->sQvga) JApplication::instance()->QvgaStart(); //
    }
    return QWidget::event(event);
}

#include "moc_jdisplay.cpp"
