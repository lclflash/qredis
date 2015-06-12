#include <QtCore>
#include "qredis.h"

class CClient :	public QObject
{
	Q_OBJECT
public:
	CClient(QObject *parent = 0);
	void timerEvent(QTimerEvent *event);
	public slots:
		void slot_subscribe(const QString &topic, const QString &data);
private:
	QRedis redis;
};

CClient::CClient(QObject *parent)
{
	redis.connectHost("127.0.0.1");
	connect(&redis, SIGNAL(subscribe(const QString &, const QString &)), this, SLOT(slot_subscribe(const QString &, const QString &)));
	redis.subscribe("t1");
	startTimer(1000);
}

void CClient::timerEvent(QTimerEvent *event)
{
	QString data = redis.get("client1");
	redis.publish("t2", QDateTime::currentDateTime().toString() + data);
}

void CClient::slot_subscribe(const QString &topic, const QString &data)
{
	qWarning() << "topic = " << topic << "\tdata" << data;
}

#include "main.moc"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB18030"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("GB18030"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB18030"));

	CClient client;

	QRedis redis;
	redis.connectHost("127.0.0.1");
//	redis.del("1");
//	redis.del(QStringList() << "2" << "µçÊÓ·Ñ");
//	redis.exists("lcl");
//	redis.expire("111", 5);
//	redis.keys("*y*");
//	redis.move("sss", 2);
//	redis.pttl("ddfdf");
//	redis.randomkey();
	redis.get("ffff");
// 	redis.lrange("key1", 0, -1);
// 	redis.publish("t1", "ffffffffffffffffffff");
// 	redis.subscribe("t1");
// 	redis.subscribe("t2");
// 	redis.subscribe("t3");
// 	redis.subscribe("t4");
// 	redis.subscribe("t5");

	return app.exec();
}
