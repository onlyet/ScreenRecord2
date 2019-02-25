#pragma once
#include <QWidget>

class QProcess;
class QFile;

class ScreenRecord : public QWidget
{
	Q_OBJECT
public:
	ScreenRecord(QWidget *parent = Q_NULLPTR);

	void Init();

	public slots:
	void Start();
	void Pause();
	void Stop();


private:
	QProcess* m_recordProcess;
	QStringList m_args;
	QString m_audioDeviceName;
	QString m_outPath;
	int m_width;
	int m_height;
	int m_fps;
	QFile* m_tmpText;
	//QByteArray m_errInfo;
	QString m_err;
	QStringList m_errList;
};
