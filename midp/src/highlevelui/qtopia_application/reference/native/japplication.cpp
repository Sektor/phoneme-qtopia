#include <QTimer>
#include <QtDebug>

#include <jvm.h>
#include <suspend_resume.h>
#include <jvmspi.h>
#include <midpEvents.h>
#include <cstdlib>

#include <japplication.h>
#include <jdisplay.h>
#include <QDesktopWidget>

#include <midp_logging.h>

#define LC_QTOPIA 10345

static void midpQtMessageHandler(QtMsgType type, const char *msg)
 {
     switch (type) {
     case QtDebugMsg:
         reportToLog(LOG_INFORMATION, LC_QTOPIA, "%s", msg);
         break;
     case QtWarningMsg:
         reportToLog(LOG_WARNING, LC_QTOPIA, "%s", msg);
         break;
     case QtCriticalMsg:
         reportToLog(LOG_ERROR, LC_QTOPIA, "%s", msg);
         break;
     case QtFatalMsg:
         reportToLog(LOG_CRITICAL, LC_QTOPIA, "%s", msg);
     }
 }


JApplication *JApplication::jApp = NULL;

JApplication::JApplication(int &argc, char **argv)
  : QtopiaApplication(argc, argv)
{
  qInstallMsgHandler(midpQtMessageHandler);
  sliceTimer.setSingleShot(true);
  vm_suspended = false;
  vm_stopped = true;
  connect(&sliceTimer, SIGNAL(timeout()), SLOT(timeSlice()));
  connect(this, SIGNAL(aboutToQuit()), SLOT(quitting()));
  runConf = new QSettings(QCoreApplication::applicationDirPath() + "/run.conf", QSettings::IniFormat);
  LoadConf();
}

JApplication::~JApplication()
{
}

void JApplication::startVM()
{
  vm_stopped = false;

  /* Setup next VM time slice to happen immediately */
  midp_resetEvents();

  scheduleTimeSlice(0);
}

void JApplication::stopVM()
{
  /* Stop any further VM time slice */
  scheduleTimeSlice(-1);
}

void JApplication::suspendVM()
{
  if (!vm_suspended)
  {
    vm_suspended = true;
    scheduleTimeSlice(-1);
  }
}

void JApplication::resumeVM()
{
  if (vm_suspended)
  {
    vm_suspended = false;
    scheduleTimeSlice(0);
  }
}

void JApplication::suspendMidp()
{
  midp_suspend();
}

void JApplication::resumeMidp()
{
  midp_resume();
}

void JApplication::scheduleTimeSlice(int millis)
{
  if (millis<0) // Negative time means that we must stop scheduling slices
    sliceTimer.stop();
  else
    sliceTimer.start(millis);
}

void JApplication::timeSlice()
{
  jlong ms;

  if (vm_stopped)
    return;

  /* check and align stack suspend/resume state */
  midp_checkAndResume();

  ms = vm_suspended ? SR_RESUME_CHECK_TIMEOUT : JVM_TimeSlice();

    /* There is a testing mode with VM running while entire stack is
     * considered to be suspended. Next invocation shold be scheduled
     * to SR_RESUME_CHECK_TIMEOUT in this case.
     */
  if (midp_getSRState() == SR_SUSPENDED)
    ms = SR_RESUME_CHECK_TIMEOUT;

  /* Let the VM run for some time */
  if (ms <= -2)
  {
    vm_stopped = true;
    exit();
  }
  else if (ms >= 0) // schedule next timeslice only if asked to
  {
    sliceTimer.start(ms);
  }
  //JDisplay::current()->repaint();
}

void JApplication::init()
{
  qDebug("JApplication initializing");
  if (jApp)
    return;
  char *argv[] = {"kvm"};
  int argc = 1;
  jApp = new JApplication(argc, argv);
  qDebug("JApplication initialized");
}

void JApplication::destroy()
{
  if (jApp)
  {
    delete jApp;
    jApp = NULL;
  }
}

void JApplication::LoadConf()
{
  cfg.sQvga = runConf->value("Display/qvga").toBool();
  cfg.sForceFullscreen = runConf->value("Display/forceFullscreen").toBool();
  cfg.sFixed = runConf->value("Display/fixed").toBool();
  cfg.sWidth = runConf->value("Display/width").toInt();
  cfg.sHeight = runConf->value("Display/height").toInt();

  cfg.tCalibrate = runConf->value("Touchscreen/calibrate").toBool();
  cfg.fNoAntiAliasing = runConf->value("Font/noAntiAliasing").toBool();
  cfg.gQuitOnHide = runConf->value("Common/quitOnHide").toBool();

  QSize screenSize = JApplication::desktop()->availableGeometry().size();
  cfg.k = (float)cfg.sWidth / (float)screenSize.width();
}

void JApplication::QvgaStart()
{
  QString start_qvga = "sh " + QCoreApplication::applicationDirPath() + "/qvga-start.sh";
  qDebug() << "Entering QVGA mode.";
  //qDebug() << start_qvga;
  system(start_qvga.toUtf8().data());
}

void JApplication::QvgaStop()
{
  QString stop_qvga = "sh " + QCoreApplication::applicationDirPath() + "/qvga-stop.sh";
  qDebug() << "Leaving QVGA mode.";
  //qDebug() << stop_qvga;
  system(stop_qvga.toUtf8().data());
}

void JApplication::quitting()
{
  if (cfg.sQvga) JApplication::instance()->QvgaStop(); //
}

#include "moc_japplication.cpp"
