#include "fscontrolpanel.h"
#include "ui_fscontrolpanel.h"
#include "fscontroller.h"
#include "fslaser.h"
#include "fsturntable.h"
#include "fsserial.h"

#include <QDebug>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

FSControlPanel::FSControlPanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FSControlPanel)
{
    ui->setupUi(this);
    this->installEventFilter(this);
}

void FSControlPanel::showEvent(QShowEvent * event)
{
    // connect to the camera frame event only if the windows is active - reduces cpu load
    QObject::connect(FSController::getInstance()->webcam, SIGNAL(cameraFrame(QImage)),this, SLOT(on_cameraFrame(QImage)));
}

void FSControlPanel::closeEvent(QCloseEvent * event)
{
    // connect to the camera frame event only if the windows is active - reduces cpu load
    QObject::disconnect(FSController::getInstance()->webcam, SIGNAL(cameraFrame(QImage)),this, SLOT(on_cameraFrame(QImage)));
}

FSControlPanel::~FSControlPanel()
{
    delete ui;
}

void FSControlPanel::on_fetchFrameButton_clicked()
{
    FSController::getInstance()->fetchFrame();
}

void FSControlPanel::on_laserOnButton_clicked()
{
    FSController::getInstance()->laser->turnOn();
}

void FSControlPanel::on_laserOffButton_clicked()
{
    FSController::getInstance()->laser->turnOff();
}

void FSControlPanel::on_checkBox_stateChanged(int state)
{
    if(state==2){
        FSController::getInstance()->turntable->enable();
    }else{
        FSController::getInstance()->turntable->disable();
    }
}

void FSControlPanel::on_stepLeftButton_clicked()
{
    FSController::getInstance()->turntable->setDirection(FS_DIRECTION_CW);
    QString str = ui->degreesEdit->text();
    double degrees = str.toDouble();
    FSController::getInstance()->turntable->turnNumberOfDegrees(degrees);
}

void FSControlPanel::on_stepRightButton_clicked()
{
    FSController::getInstance()->turntable->setDirection(FS_DIRECTION_CCW);
    QString str = ui->degreesEdit->text();
    double degrees = str.toDouble();
    FSController::getInstance()->turntable->turnNumberOfDegrees(degrees);
}

void FSControlPanel::on_autoResetButton_clicked()
{
    if(FSController::getInstance()->webcam->info.portName.isEmpty()){
        FSController::getInstance()->mainwindow->showDialog("No webcam selected!");
        return;
    }
    FSController::getInstance()->detectLaserLine();
    cv::Mat shot = FSController::getInstance()->webcam->getFrame();
    cv::resize( shot,shot,cv::Size(1280,960) );
    shot = FSController::getInstance()->vision->drawLaserLineToFrame(shot);
    cv::resize(shot,shot,cv::Size(800,600));
    cv::imshow(WINDOW_LASER_FRAME,shot);
    this->raise();
    this->focusWidget();
    this->setVisible(true);
}

void FSControlPanel::on_laserEnable_stateChanged(int state)
{
    if(state==2){
        FSController::getInstance()->laser->enable();
    }else{
        FSController::getInstance()->laser->disable();
    }
}

void FSControlPanel::on_laserStepLeftButton_clicked()
{
    FSController::getInstance()->laser->setDirection(FS_DIRECTION_CCW);
    FSController::getInstance()->laser->turnNumberOfDegrees(2.0);
}

void FSControlPanel::on_laserStepRightButton_clicked()
{
    FSController::getInstance()->laser->setDirection(FS_DIRECTION_CW);
    FSController::getInstance()->laser->turnNumberOfDegrees(2.0);
}

void FSControlPanel::on_diffImage_clicked()
{
    if(FSController::getInstance()->webcam->info.portName.isEmpty()){
        FSController::getInstance()->mainwindow->showDialog("No webcam selected!");
        return;
    }
    cv::Mat shot = FSController::getInstance()->diffImage();
    cv::resize(shot,shot,cv::Size(800,600));
    cv::imshow(WINDOW_LASER_FRAME,shot);

    this->raise();
    this->focusWidget();
    this->setVisible(true);
}

void FSControlPanel::on_laserSwipeMaxEdit_returnPressed()
{
    FSController::getInstance()->laserSwipeMax = (ui->laserSwipeMaxEdit->text()).toDouble();
}

void FSControlPanel::setLaserAngleText(double angle)
{
    QString a = QString("Angle: %1º").arg(angle);
    this->ui->laserAngle->setText(a);
}

void FSControlPanel::on_laserSwipeMinEdit_returnPressed()
{
    FSController::getInstance()->laserSwipeMin = (ui->laserSwipeMinEdit->text()).toDouble();
}

void FSControlPanel::on_pushButton_2_clicked()
{

    FSController::getInstance()->laser->enable();
    FSController::getInstance()->laser->turnOff();
    QThread::msleep(200);
    cv::Mat laserOffFrame = FSController::getInstance()->webcam->getFrame();
    FSController::getInstance()->laser->turnOn();
    QThread::msleep(200);
    cv::Mat laserOnFrame = FSController::getInstance()->webcam->getFrame();
    cv::resize( laserOnFrame,laserOnFrame,cv::Size(1280,960) );
    cv::resize( laserOffFrame,laserOffFrame,cv::Size(1280,960) );

    qDebug() << "pressed";
    cv::Mat shot = FSVision::subLaser2(laserOffFrame, laserOnFrame);
    cv::resize(shot,shot,cv::Size(800,600));
    cv::imshow("Laser Frame",shot);
    cv::waitKey(0);
    cvDestroyWindow("Laser Frame");
    this->raise();
    this->focusWidget();
    this->setVisible(true);
}

void FSControlPanel::on_cameraFrame(QImage frame)
{
    // We've received a frame from CV VideoCapture::read() via fswebcam
    // It is a QImage structure.
    int width;

    width = ui->cameraLabel->width();	//We have to scale it down to fit the preview window, so get the window's width
    ui->cameraLabel->setPixmap(QPixmap::fromImage(frame.scaledToWidth(width, Qt::FastTransformation))); // Show it
}
