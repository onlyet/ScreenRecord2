#include "ScreenRecord2.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QFile>

#include <Windows.h>

ScreenRecord::ScreenRecord(QWidget *parent)
	: QWidget(parent)
	, m_process(new QProcess(this))
{
	Init();
	QTimer::singleShot(1000, this, SLOT(Start()));
	QTimer::singleShot(5000, this, SLOT(Pause()));

	QTimer::singleShot(6000, this, SLOT(Start()));
	QTimer::singleShot(10000, this, SLOT(Stop()));
}

void ScreenRecord::Init()
{
	QProcess *pro = new QProcess(this);
	connect(pro, &QProcess::readyReadStandardError, [pro, this]() {
		QString tmp = pro->readAllStandardError();
		m_errInfo.append(tmp);
		qDebug() << tmp;
	});
	QStringList tmplist;
	tmplist << "-list_devices" << "true" << "-f" << "dshow" << "-i" << "dummy";
	pro->start("ffmpeg", tmplist);
	pro->waitForFinished();
	//qDebug() << pro->readAllStandardOutput();
	//qDebug() << pro->readAllStandardError();
	//pro->readyReadStandardError();

	m_outPath = "D:/test.mp4";
	m_width = QApplication::desktop()->screenGeometry().width();
	m_height = QApplication::desktop()->screenGeometry().height();
	m_fps = 25;
	//m_audioDeviceName = "麦克风 (High Definition Audio 设备)";
	QString audioName = "audio=" + QString::fromLocal8Bit("麦克风 (High Definition Audio 设备)");

	m_args << "-f" << "gdigrab";
	m_args << "-i" << "desktop";
	m_args << "-f" << "dshow";
	m_args << "-i" << audioName;
	m_args << "-pix_fmt" << "yuv420p";
	m_args << "-vcodec" << "libx264";
	m_args << "-acodec" << "aac";
	m_args << "-s" << QString::number(m_width) + "x" + QString::number(m_height);
	m_args << "-r" << QString::number(m_fps);
	m_args << "-q" << "10";
	m_args << "-ar" << "44100";
	m_args << "-ac" << "2";
	m_args << "-tune" << "zerolatency";
	m_args << "-preset" << "ultrafast";
	m_args << "-f" << "mp4";

	m_tmpText = new QFile(QString("D:\\t%1_tmp").arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
	if (!m_tmpText->open(QIODevice::ReadWrite | QIODevice::Text))
		qDebug() << "File open error";

	qDebug() << "text name:" << m_tmpText->fileName();
	//m_args << m_outPath;
}

void ScreenRecord::Start()
{
	//反斜线对应文本中是绝对路径，斜线对应相对路径
	QString tmpFilePath = QString("v%1_tmp").arg(QDateTime::currentDateTime().toMSecsSinceEpoch());

	QTextStream out(m_tmpText);
	out << "file" << " '" << tmpFilePath << "'\n";

	QStringList args = m_args;
	QString tmpFileAbsolutePath = "D:\\" + tmpFilePath;
	args << tmpFileAbsolutePath;
	m_process->start("ffmpeg", args);
}

void ScreenRecord::Pause()
{
	m_process->write("q");
}

void ScreenRecord::Stop()
{
	//qDebug() << m_process->readAllStandardError();

	m_process->write("q");
	m_tmpText->close();

	m_process->waitForFinished();

	QProcess* pro = new QProcess(this);
	QStringList args;
	//ffmpeg -f concat -safe 0 -i filelist.txt -c copy output.mkv
	args << "-f" << "concat";
	args << "-safe" << "0";	
	args << "-i" << m_tmpText->fileName();
	args << "-c" << "copy";
	args << m_outPath;

	pro->start("ffmpeg", args);
	pro->waitForFinished();
	//qDebug() << "stop";
	//qDebug() << pro->readAllStandardError();
}