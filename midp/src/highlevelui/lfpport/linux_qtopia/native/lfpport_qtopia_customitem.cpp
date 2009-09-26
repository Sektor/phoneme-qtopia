/*
 *
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 *
 * This source file is specific for Qtopia4-based configurations.
 * Email: trollsid@gmail.com
 */

#include "lfpport_qtopia_pcsl_string.h"

#include <lfpport_customitem.h>
#include "lfpport_qtopia_customitem.h"
#include "lfpport_qtopia_debug.h"
#include "lfpport_qtopia_displayable.h"
#include <jgraphics.h>
#include <midpEventUtil.h>
#include <midpEvents.h>
#include <jkey.h>
#include <keymap_input.h>
#include <jdisplay.h>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QPixmap>
#include <QEvent>
#include <QFocusEvent>
#include <Qt>
#include <QMouseEvent>
#include <jdisplay.h>
#warning BEWARE of stubs!

#define PAD_SIZE 4

extern "C"
{

  MidpError lfpport_customitem_create(MidpItem* itemPtr,
                                      MidpDisplayable* ownerPtr,
                                      const pcsl_string* label, int layout)
  {
    debug_trace();
    JDisplayable *disp = static_cast<JDisplayable *>(ownerPtr->frame.widgetPtr);
    if(disp == NULL)
    {
        return KNI_ENOMEM;
    }
    qDebug() << "Create custemItem";
    JCustomItem *cItem = new JCustomItem(itemPtr, disp->toForm(), pcsl_string2QString(*label));
    qDebug() << "Created";
    return KNI_OK;
  }

  MidpError lfpport_customitem_refresh(MidpItem* itemPtr,
                                       int x, int y,
                                       int width, int height)
  {
        JCustomItem *item = static_cast<JCustomItem *>(itemPtr->widgetPtr);
        item->j_refreshSurface(x, y, width, height);
        qDebug() << "refresh";
        return KNI_OK;
  }

  MidpError lfpport_customitem_get_label_width(int *widthRet,
                                               int width,
                                               MidpItem* ciPtr)
  {
      (void)width;
      JCustomItem *item = static_cast<JCustomItem *>(ciPtr->widgetPtr);
      *widthRet = item->getLabelWidth();
      return KNI_OK;
  }

  MidpError lfpport_customitem_get_label_height(int width,
      int *heightRet,
      MidpItem* ciPtr)
  {
        (void)width;
        qDebug() << "label height";
        JCustomItem *item = static_cast<JCustomItem *>(ciPtr->widgetPtr);
        *heightRet = item->getLabelHeight();
        qDebug() << "label height";
        return KNI_OK;
  }

  MidpError lfpport_customitem_get_item_pad(int *pad, MidpItem* ciPtr)
  {
		*pad = PAD_SIZE; 
		return KNI_OK;
  }

  MidpError lfpport_customitem_set_content_buffer(MidpItem* ciPtr,
      unsigned char* imgPtr)
  {
      JCustomItem *item = static_cast<JCustomItem *>(ciPtr->widgetPtr);
      item->j_setContentBuffer(imgPtr);
      return KNI_OK;
  }
}
///*


JCustomItemSurface::JCustomItemSurface(QWidget *parent)
  : QWidget(parent)
{
    canvas = NULL;
    setFocusPolicy(Qt::StrongFocus);
}

JCustomItemSurface::~JCustomItemSurface()
{
}

void JCustomItemSurface::mousePressEvent(QMouseEvent *event)
{
    MidpEvent ev;
    MIDP_EVENT_INITIALIZE(ev);
    ev.type = MIDP_PEN_EVENT;
    ev.ACTION = KEYMAP_STATE_PRESSED;
    ev.X_POS = event->x();
    ev.Y_POS = event->y();
    midpStoreEventAndSignalForeground(ev);
}

void JCustomItemSurface::mouseReleaseEvent(QMouseEvent *event)
{
    MidpEvent ev;
    MIDP_EVENT_INITIALIZE(ev);
    ev.type = MIDP_PEN_EVENT;
    ev.ACTION = KEYMAP_STATE_RELEASED;
    ev.X_POS = event->x();
    ev.Y_POS = event->y();
    midpStoreEventAndSignalForeground(ev);
}


void JCustomItemSurface::setCanvas(QPixmap *p)
{
    canvas = p;
}

bool JCustomItemSurface::event(QEvent *event)
{
	{
		return QWidget::event(event);
	}
}

void JCustomItemSurface::paintEvent(QPaintEvent *ev)
{
    QRect r(ev->rect());
    if(canvas != NULL)
    {
        QPainter painter(this);
        painter.drawPixmap(ev->rect(), *canvas);
    }
}

void JCustomItemSurface::keyPressEvent(QKeyEvent *event)
{
    MidpEvent midp_event;
    MIDP_EVENT_INITIALIZE(midp_event);
    if(LFPKeyMap::instance()->map(event->key(), event->text(), midp_event.CHR))
    {
        midp_event.type = MIDP_KEY_EVENT;
        midp_event.ACTION = KEYMAP_STATE_PRESSED;
        midpStoreEventAndSignalForeground(midp_event);
    }
}

void JCustomItemSurface::keyReleaseEvent(QKeyEvent *event)
{
  
  MidpEvent midp_event;
  MIDP_EVENT_INITIALIZE(midp_event);
  if(LFPKeyMap::instance()->map(event->key(), event->text(), midp_event.CHR))
  {
    midp_event.type = MIDP_KEY_EVENT;
    midp_event.ACTION = KEYMAP_STATE_RELEASED;
    midpStoreEventAndSignalForeground(midp_event);
  }
}

void JCustomItemSurface::refreshSurface(int x, int y, int w, int h)
{
    if(canvas != NULL)
    {
        QPainter painter(this);//(canvas);
        painter.drawPixmap(x, y, w , h, *canvas);
    }
}

//*/
//===================
//
JCustomItem::JCustomItem(MidpItem *item, JForm *form, const QString label)
  : JItem(item, form)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    w_label = new QLabel(this);
    w_label->setText(label);
    surface = new JCustomItemSurface;
    setFocusPolicy(Qt::TabFocus);
    setLayout(layout);
    if(!label.isNull())
    {
        layout->addWidget(w_label);
    }
    layout->addWidget(surface);
}

JCustomItem::~JCustomItem()
{
}

void JCustomItem::j_setLabel(const QString &text)
{
    w_label->setText(text);
}

void JCustomItem::focusInEvent(QFocusEvent *event)
{
    if(event->reason() != Qt::OtherFocusReason)
    {
        MidpFormFocusChanged(this);
    }
    JItem::focusInEvent(event);
}

QSize JCustomItem::j_getLabelSize()
{
    return w_label->size();
}

void JCustomItem::j_refreshSurface(int x, int y, int w, int h)
{
    surface->refreshSurface(x, y, w, h);
}

void JCustomItem::j_setContentBuffer(unsigned char *buffer)
{
    if(buffer != NULL)
    {
            QPixmap *pix = gxpportqt_get_mutableimage_pixmap(buffer);
            surface->setCanvas(pix);
    }
}

bool JCustomItem::event(QEvent *event)
{
    JItem::event(event);
    return true;
}

int JCustomItem::j_getItemPad()
{
    return 0;
}

int JCustomItem::getLabelHeight()
{
    return w_label->height();
}

int JCustomItem::getLabelWidth()
{
    return w_label->width();
}
//*/

#include "lfpport_qtopia_customitem.moc"
