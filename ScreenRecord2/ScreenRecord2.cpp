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
	, m_recordProcess(new QProcess(this))
{
	connect(m_recordProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
		[this](int exitCode, QProcess::ExitStatus exitStatus) {
		qDebug() << QStringLiteral("成功录制临时视频");
	});

	Init();
	QTimer::singleShot(1000, this, SLOT(Start()));
	QTimer::singleShot(5000, this, SLOT(Pause()));

	QTimer::singleShot(6000, this, SLOT(Start()));
	QTimer::singleShot(10000, this, SLOT(Stop()));
}

void ScreenRecord::Init()
{
	QProcess *listProcess = new QProcess(this);
	connect(listProcess, &QProcess::readyReadStandardError, [listProcess, this]() {
		m_err.append(listProcess->readAllStandardError());
	});
	connect(listProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
		[this](int exitCode, QProcess::ExitStatus exitStatus) {
		m_errList = m_err.split("\r\n", QString::SkipEmptyParts);
		QStringList filter = m_errList.filter(QString::fromLocal8Bit("麦克风"));
		QString microphone = filter.first();
		QStringList filter2 = microphone.split("\"", QString::SkipEmptyParts);
		QStringList filter3 = filter2.filter(QString::fromLocal8Bit("麦克风"));
		//"麦克风 (High Definition Audio 设备)";
		m_audioDeviceName = "audio=" + filter3.first();

		m_outPath = QString("%1.mp4").arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
		m_width = QApplication::desktop()->screenGeometry().width();
		m_height = QApplication::desktop()->screenGeometry().height();
		m_fps = 25;
		//QString audioName = "audio=" + QString::fromLocal8Bit("麦克风 (High Definition Audio 设备)");
		m_args << "-f" << "gdigrab";
		m_args << "-i" << "desktop";
		m_args << "-f" << "dshow";
		m_args << "-i" << m_audioDeviceName;
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
	});
	QStringList listCmdArgs;
	listCmdArgs << "-list_devices" << "true" << "-f" << "dshow" << "-i" << "dummy";
	listProcess->start("ffmpeg", listCmdArgs);
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
	m_recordProcess->start("ffmpeg", args);
}

void ScreenRecord::Pause()
{
	//暂停录制->生成临时视频
	m_recordProcess->write("q");
}

void ScreenRecord::Stop()
{
	//结束录制->合成视频
	connect(m_recordProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
		[this](int exitCode, QProcess::ExitStatus exitStatus) {
		QProcess* mergeProcess = new QProcess(this);
		connect(mergeProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
			[mergeProcess, this](int exitCode, QProcess::ExitStatus exitStatus) {
			qDebug() << QStringLiteral("成功合成视频");
			qDebug() << mergeProcess->readAllStandardError();
		});
		//ffmpeg -f concat -safe 0 -i filelist.txt -c copy output.mp4
		QStringList args;
		args << "-f" << "concat";
		args << "-safe" << "0";
		args << "-i" << m_tmpText->fileName();
		args << "-c" << "copy";
		args << m_outPath;
		//合成视频
		mergeProcess->start("ffmpeg", args);
	});

	m_tmpText->close();
	m_recordProcess->write("q");
}