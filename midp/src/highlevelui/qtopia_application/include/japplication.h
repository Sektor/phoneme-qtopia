#ifndef _JAPPLICATION_H_
#define _JAPPLICATION_H_

#include <QtopiaApplication>
#include <QTimer>
#include <QSettings>

struct RunConfig
{
  bool sQvga;
  bool sForceFullscreen;
  bool sFixed;
  int sWidth;
  int sHeight;
  float k;

  bool tCalibrate;
  bool fNoAntiAliasing;
  bool gQuitOnHide;
};

class JApplication: public QtopiaApplication
{
  Q_OBJECT
  public:
    JApplication(int &argc, char **argv);
    virtual ~JApplication();

    RunConfig cfg;

    void QvgaStart();
    void QvgaStop();

     /**
      * Start to give VM time slice to run.
      */
    void startVM();

    /**
     * Stop VM from any further time slice.
     * Any UI leftover resource will be freed.
     */
    void stopVM();

    /**
     * Suspend VM. VM will not receive time slices until resumed.
     */
    void suspendVM();

    /**
     * Resume VM to normal operation.
     */
    void resumeVM();

    /**
     * Suspend VM. VM will not receive time slices until resumed.
     */
    void suspendMidp();

    /**
     * Resume VM to normal operation.
     */
    void resumeMidp();

    /**
     * Trigger VM timeslice in \a millis milliseconds
     */
    void scheduleTimeSlice(int millis);

    inline static JApplication *instance() { return jApp; }
    static void init();
    static void destroy();
  private slots:
    /**
     * Perform VM timeslice
     */
    void timeSlice();
    void quitting();
  private:
    QTimer sliceTimer;
    bool vm_stopped;
    bool vm_suspended;
    static JApplication *jApp;

    QSettings *runConf;
    void LoadConf();
};

#endif // _JAPPLICATION_H_
