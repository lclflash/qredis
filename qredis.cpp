#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include "qredis.h"

redis_reply::redis_reply(QObject *parent) : QObject(parent), reply_type_(REDIS_RESULT_NIL)
{

}

redis_reply_t redis_reply::type()
{
	return reply_type_;
}

qlonglong redis_reply::integer()
{
	if (reply_type_ != REDIS_RESULT_INTEGER)
		return -1;
	return str.toLongLong();
}

QString redis_reply::status()
{
	if (reply_type_ != REDIS_RESULT_STATUS)
		return "";
	return str;
}

QString redis_reply::error()
{
	if (reply_type_ != REDIS_RESULT_ERROR)
		return "";
	return str;
}

QString redis_reply::string()
{
	if (reply_type_ != REDIS_RESULT_STRING)
		return "";
	return str;
}

void redis_reply::setData(const QString &data)
{
	str = data;
}

void redis_reply::setType(redis_reply_t type)
{
	reply_type_ = type;
}

QRedis::QRedis(QObject * parent) : QObject(parent)
{
	m_isconnected = false;

	m_sock = new QTcpSocket(this);
	connect(m_sock, SIGNAL(connected()), this, SLOT(connected()));
	connect(m_sock, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

	m_subssock = new QTcpSocket(this);
	connect(m_subssock, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(m_subssock, SIGNAL(connected()), this, SLOT(connected()));
	connect(m_subssock, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(m_subssock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(check()));
	timer->start(5000);
}

QRedis::~QRedis()
{
}

void QRedis::connectHost(const QString & hostName, quint16 port)
{
	m_ip = hostName;
	m_port = port;
	m_sock->connectToHost(hostName, port);
	if (!m_sock->waitForConnected(1000))
	{
		m_sock->disconnectFromHost();
		return;
	}

	m_subssock->connectToHost(hostName, port);
	if (!m_subssock->waitForConnected(1000))
	{
		m_subssock->disconnectFromHost();
		return;
	}
}

int QRedis::del(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("del");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

int QRedis::del(const QStringList &keys)
{
	QList<QByteArray>temp;
	temp.append("del");
	foreach(QString key, keys)
	{
		temp.append(key.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

bool QRedis::exists(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("exists");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::expire(const QString &key, qlonglong secs)
{
	QList<QByteArray>temp;
	temp.append("expire");
	temp.append(key.toUtf8());
	temp.append(QString::number(secs).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::expireat(const QString &key, qlonglong timestamp)
{
	QList<QByteArray>temp;
	temp.append("expireat");
	temp.append(key.toUtf8());
	temp.append(QString::number(timestamp).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

QStringList QRedis::keys(const QString &pattern)
{
	QList<QByteArray>temp;
	temp.append("keys");
	temp.append(pattern.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

bool QRedis::move(const QString &key, int db)
{
	QList<QByteArray>temp;
	temp.append("move");
	temp.append(key.toUtf8());
	temp.append(QString::number(db).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::persist(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("persist");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::pexpire(const QString &key, qlonglong mils)
{
	QList<QByteArray>temp;
	temp.append("pexpire");
	temp.append(key.toUtf8());
	temp.append(QString::number(mils).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::pexpireat(const QString &key, qlonglong milstimestamp)
{
	QList<QByteArray>temp;
	temp.append("pexpireat");
	temp.append(key.toUtf8());
	temp.append(QString::number(milstimestamp).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

qlonglong QRedis::pttl(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("pttl");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

QString QRedis::randomkey()
{
	QList<QByteArray>temp;
	temp.append("randomkey");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

bool QRedis::rename(const QString &key, const QString &newkey)
{
	QList<QByteArray>temp;
	temp.append("rename");
	temp.append(key.toUtf8());
	temp.append(newkey.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::renamenx(const QString &key, const QString &newkey)
{
	QList<QByteArray>temp;
	temp.append("renamenx");
	temp.append(key.toUtf8());
	temp.append(newkey.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

qlonglong QRedis::ttl(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("ttl");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

QString QRedis::type(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("type");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

int QRedis::append(const QString &key, const QString &value)
{
	QList<QByteArray>temp;
	temp.append("append");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::decr(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("decr");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return -9999;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return -9999;
}

qlonglong QRedis::decrby(const QString &key, qlonglong value)
{
	QList<QByteArray>temp;
	temp.append("decrby");
	temp.append(key.toUtf8());
	temp.append(QString::number(value).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return -9999;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return -9999;
}

QString QRedis::get(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("get");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	//QByteArray data;
	//QCoreApplication::processEvents();
	//while (m_sock->bytesAvailable())
	//{
	//	data += m_sock->readAll();
	//	m_sock->waitForReadyRead(1000);
	//}

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

QString QRedis::getrange(const QString &key, qlonglong start, qlonglong stop)
{
	QList<QByteArray> temp;
	temp.append("getrange");
	temp.append(key.toUtf8());
	temp.append(QString::number(start).toUtf8());
	temp.append(QString::number(stop).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

QString QRedis::getset(const QString &key, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("getset");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

qlonglong QRedis::incr(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("incr");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return -9999;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return -9999;
}

qlonglong QRedis::incrby(const QString &key, qlonglong value)
{
	QList<QByteArray>temp;
	temp.append("incrby");
	temp.append(key.toUtf8());
	temp.append(QString::number(value).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return -9999;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return -9999;
}

qreal QRedis::incrbyfloat(const QString &key, qreal value)
{
	QList<QByteArray>temp;
	temp.append("incrbyfloat");
	temp.append(key.toUtf8());
	temp.append(QString::number(value).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0.0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string().toDouble();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0.0;
}

QStringList QRedis::mget(const QStringList &keys)
{
	QList<QByteArray>temp;
	temp.append("mget");
	foreach(QString key, keys)
	{
		temp.append(key.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

void QRedis::mset(const QStringList &keyvalues)
{
	QList<QByteArray>temp;
	temp.append("mset");
	foreach(QString kv, keyvalues)
	{
		temp.append(kv.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}
}

bool QRedis::msetnx(const QStringList &keyvalues)
{
	QList<QByteArray>temp;
	temp.append("msetnx");
	foreach(QString kv, keyvalues)
	{
		temp.append(kv.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::psetex(const QString &key, qlonglong mils, const QString &value)
{
	QList<QByteArray>temp;
	temp.append("psetex");
	temp.append(QString::number(mils).toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::set(const QString &key, const QString &value)
{
	QList<QByteArray>temp;
	temp.append("set");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS && (rr->string() == "OK"))
	{
		return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::setex(const QString &key, qlonglong secs, const QString &value)
{
	QList<QByteArray>temp;
	temp.append("setex");
	temp.append(key.toUtf8());
	temp.append(QString::number(secs).toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS && (rr->string() == "OK"))
	{
		return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::setnx(const QString &key, const QString &value)
{
	QList<QByteArray>temp;
	temp.append("setnx");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER )
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

qlonglong QRedis::setrange(const QString &key, qlonglong offset, const QString &value)
{
	QList<QByteArray>temp;
	temp.append("setrange");
	temp.append(key.toUtf8());
	temp.append(QString::number(offset).toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER )
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::strlen(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("strlen");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER )
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::hdel(const QString &key, const QString &field)
{
	QList<QByteArray>temp;
	temp.append("hdel");
	temp.append(key.toUtf8());
	temp.append(field.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER )
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::hdel(const QString &key, const QStringList &fields)
{
	QList<QByteArray>temp;
	temp.append("hdel");
	temp.append(key.toUtf8());
	foreach(QString field, fields)
	{
		temp.append(field.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER )
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

bool QRedis::hexists(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("hexists");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER )
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

QString QRedis::hget(const QString &key, const QString &field)
{
	QList<QByteArray>temp;
	temp.append("hget");
	temp.append(key.toUtf8());
	temp.append(field.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

QStringList QRedis::hgetall(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("hgetall");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

qlonglong QRedis::hincrby(const QString &key, const QString &field, qlonglong value)
{
	QList<QByteArray>temp;
	temp.append("hincrby");
	temp.append(key.toUtf8());
	temp.append(field.toUtf8());
	temp.append(QString::number(value).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return -9999;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return -9999;
}

qreal QRedis::hincrbyfloat(const QString &key, const QString &field, qreal value)
{
	QList<QByteArray>temp;
	temp.append("hincrbyfloat");
	temp.append(key.toUtf8());
	temp.append(field.toUtf8());
	temp.append(QString::number(value).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0.0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string().toDouble();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0.0;
}

QStringList QRedis::hkeys(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("hkeys");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

qlonglong QRedis::hlen(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("hlen");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return -9999;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return -9999;
}

QStringList QRedis::hmget(const QString &key, const QStringList &fields)
{
	QList<QByteArray>temp;
	temp.append("hmget");
	temp.append(key.toUtf8());
	foreach(QString field, fields)
	{
		temp.append(field.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

bool QRedis::hmset(const QString &key, const QStringList &fvs)
{
	QList<QByteArray> temp;
	temp.append("hmset");
	temp.append(key.toUtf8());
	foreach(QString fv, fvs)
	{
		temp.append(fv.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

int QRedis::hset(const QString &key, const QString &field, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("hset");
	temp.append(key.toUtf8());
	temp.append(field.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return -1;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return -1;
}

bool QRedis::hsetnx(const QString &key, const QString &field, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("hsetnx");
	temp.append(key.toUtf8());
	temp.append(field.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

QStringList QRedis::hvals(const QString &key)
{
	QList<QByteArray>temp;
	temp.append("hvals");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

QString QRedis::lindex(const QString &key, qlonglong index)
{
	QList<QByteArray> temp;
	temp.append("lindex");
	temp.append(key.toUtf8());
	temp.append(QString::number(index).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

qlonglong QRedis::llen(const QString &key)
{
	QList<QByteArray> temp;
	temp.append("llen");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

QString QRedis::lpop(const QString &key)
{
	QList<QByteArray> temp;
	temp.append("lpop");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

qlonglong QRedis::lpush(const QString &key, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("lpush");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::lpush(const QString &key, const QStringList &values)
{
	QList<QByteArray> temp;
	temp.append("lpush");
	temp.append(key.toUtf8());
	foreach(QString value, values)
	{
		temp.append(value.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

QStringList QRedis::lrange(const QString &key, qlonglong start, qlonglong stop)
{
	QList<QByteArray> temp;
	temp.append("lrange");
	temp.append(key.toUtf8());
	temp.append(QString::number(start).toUtf8());
	temp.append(QString::number(stop).toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	QStringList data;
	redis_reply *rr = get_redis_object(m_sock);
	if (!rr) return data;

	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

qlonglong QRedis::lrem(const QString &key, int count, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("lrem");
	temp.append(key.toUtf8());
	temp.append(QString::number(count).toLocal8Bit());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

bool QRedis::lset(const QString &key, int index, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("lset");
	temp.append(key.toUtf8());
	temp.append(QString::number(index).toUtf8());
	temp.append(value.toLocal8Bit());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

QString QRedis::rpop(const QString &key)
{
	QList<QByteArray> temp;
	temp.append("rpop");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

qlonglong QRedis::rpush(const QString &key, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("rpush");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::rpush(const QString &key, const QStringList &values)
{
	QList<QByteArray> temp;
	temp.append("rpush");
	temp.append(key.toUtf8());
	foreach(QString value, values)
	{
		temp.append(value.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::sadd(const QString &key, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("sadd");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::sadd(const QString &key, const QStringList &values)
{
	QList<QByteArray> temp;
	temp.append("sadd");
	temp.append(key.toUtf8());
	foreach(QString value, values)
	{
		temp.append(value.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::scard(const QString &key)
{
	QList<QByteArray> temp;
	temp.append("scard");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

QStringList QRedis::sdiff(const QStringList &keys)
{
	QList<QByteArray> temp;
	temp.append("sdiff");
	foreach(QString key, keys)
	{
		temp.append(key.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

QStringList QRedis::sinter(const QStringList &keys)
{
	QList<QByteArray> temp;
	temp.append("sinter");
	foreach(QString key, keys)
	{
		temp.append(key.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

bool QRedis::sismember(const QString &key, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("sismember");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

QStringList QRedis::smembers(const QString &key)
{
	QList<QByteArray> temp;
	temp.append("smembers");
	temp.append(key.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

qlonglong QRedis::srem(const QString &key, const QString &value)
{
	QList<QByteArray> temp;
	temp.append("srem");
	temp.append(key.toUtf8());
	temp.append(value.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

qlonglong QRedis::srem(const QString &key, const QStringList &values)
{
	QList<QByteArray> temp;
	temp.append("srem");
	temp.append(key.toUtf8());
	foreach(QString value, values)
	{
		temp.append(value.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

QStringList QRedis::sunion(const QStringList &keys)
{
	QList<QByteArray> temp;
	temp.append("sunion");
	foreach(QString key, keys)
	{
		temp.append(key.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

void QRedis::psubscribe(const QString &pattern)
{
	QList<QByteArray> temp;
	temp.append("psubscribe");
	temp.append(pattern.toUtf8());
	m_pchannels.insert(pattern);

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();
}

void QRedis::psubscribe(const QStringList &patterns)
{
	QList<QByteArray> temp;
	temp.append("psubscribe");
	foreach(QString p, patterns)
	{
		temp.append(p.toUtf8());
		m_pchannels.insert(p);
	}

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();
}

int QRedis::publish(const QString &channel, const QString &data)
{
	QList<QByteArray> temp;
	temp.append("publish");
	temp.append(channel.toUtf8());
	temp.append(data.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

void QRedis::punsubscribe()
{
	QList<QByteArray> temp;
	temp.append("punsubscribe");

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();

	m_pchannels.clear();
}

void QRedis::punsubscribe(const QString &pattern)
{
	QList<QByteArray> temp;
	temp.append("punsubscribe");
	temp.append(pattern.toUtf8());

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();

	m_pchannels.remove(pattern);
}

void QRedis::punsubscribe(const QStringList &patterns)
{
	QList<QByteArray> temp;
	temp.append("punsubscribe");
	foreach(QString p, patterns)
	{
		temp.append(p.toUtf8());
		m_pchannels.remove(p);
	}

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();
}

void QRedis::subscribe(const QString &channel)
{
	QList<QByteArray> temp;
	temp.append("subscribe");
	temp.append(channel.toUtf8());
	m_channels.insert(channel);

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();
}

void QRedis::subscribe(const QStringList &channels)
{
	QList<QByteArray> temp;
	temp.append("subscribe");
	foreach(QString c, channels)
	{
		temp.append(c.toUtf8());
		m_channels.insert(c);
	}

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();
}

void QRedis::unsubscribe()
{
	QList<QByteArray> temp;
	temp.append("unsubscribe");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	m_channels.clear();
}

void QRedis::unsubscribe(const QString &channel)
{
	QList<QByteArray> temp;
	temp.append("unsubscribe");
	temp.append(channel.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	m_channels.remove(channel);
}

void QRedis::unsubscribe(const QStringList &channels)
{
	QList<QByteArray> temp;
	temp.append("unsubscribe");
	foreach(QString c, channels)
	{
		temp.append(c.toUtf8());
		m_channels.remove(c);
	}

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();
}

QStringList QRedis::eval(const QString &script, const QStringList &args)
{
	QList<QByteArray> temp;
	temp.append("eval");
	temp.append(script.toUtf8());
	foreach(QString arg, args)
	{
		temp.append(arg.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

QStringList QRedis::evalsha(const QString &sha1, const QStringList &args)
{
	QList<QByteArray> temp;
	temp.append("evalsha");
	temp.append(sha1.toUtf8());
	foreach(QString arg, args)
	{
		temp.append(arg.toUtf8());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

bool QRedis::scriptexists(const QString &sha1)
{
	QList<QByteArray> temp;
	temp.append("script");
	temp.append("exists");
	temp.append(sha1.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

QStringList QRedis::scriptexists(const QStringList &sha1s)
{
	QList<QByteArray> temp;
	temp.append("script");
	temp.append("exists");
	foreach(QString arg, sha1s)
	{
		temp.append(arg.toLocal8Bit());
	}

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

void QRedis::scriptflush()
{
	QList<QByteArray> temp;
	temp.append("script");
	temp.append("flush");

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
}

void QRedis::scriptkill()
{
	QList<QByteArray> temp;
	temp.append("script");
	temp.append("kill");

	QByteArray res = format(temp);
	m_subssock->write(res);
	m_subssock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
}

QString QRedis::scriptload(const QString &script)
{
	QList<QByteArray> temp;
	temp.append("script");
	temp.append("load");
	temp.append(script.toLocal8Bit());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

bool QRedis::auth(const QString &pw)
{
	QList<QByteArray> temp;
	temp.append("auth");
	temp.append(pw.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::ping()
{
	QList<QByteArray> temp;
	temp.append("ping");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		if (rr->string() == "PONG") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

void QRedis::quit()
{
	QList<QByteArray> temp;
	temp.append("quit");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
}

bool QRedis::select(int db)
{
	QList<QByteArray> temp;
	temp.append("select");
	temp.append(QString::number(db).toLocal8Bit());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

bool QRedis::bgsave()
{
	QList<QByteArray> temp;
	temp.append("dbsize");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

QString QRedis::clientgetname()
{
	QList<QByteArray> temp;
	temp.append("client getname");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

bool QRedis::clientkill(const QString &ipport)
{
	QList<QByteArray> temp;
	temp.append("client kill");
	temp.append(ipport.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

QStringList QRedis::clientlist()
{
	QList<QByteArray> temp;
	temp.append("client list");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QStringList();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return data;
}

bool QRedis::clientsetname(const QString &name)
{
	QList<QByteArray> temp;
	temp.append("client setname");
	temp.append(name.toUtf8());

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return false;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STATUS)
	{
		if (rr->string() == "OK") return true;
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return false;
}

qlonglong QRedis::dbsize()
{
	QList<QByteArray> temp;
	temp.append("dbsize");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return 0;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_INTEGER)
	{
		return rr->integer();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return 0;
}

void QRedis::flushall()
{
	QList<QByteArray> temp;
	temp.append("flushall");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
}

void QRedis::flushdb()
{
	QList<QByteArray> temp;
	temp.append("flushdb");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return;
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();
}

QString QRedis::info()
{
	QList<QByteArray> temp;
	temp.append("info");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return "";
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	if (rr->type() == REDIS_RESULT_STRING)
	{
		return rr->string();
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	return "";
}

QDateTime QRedis::time()
{
	QList<QByteArray> temp;
	temp.append("time");

	QByteArray res = format(temp);
	m_sock->write(res);
	m_sock->flush();

	bool ret = m_sock->waitForReadyRead(1000);
	if (!ret)
	{
		m_error = "read time out";
		return QDateTime();
	}

	redis_reply *rr = get_redis_object(m_sock);
	rr->deleteLater();

	QStringList data;
	if (rr->type() == REDIS_RESULT_ARRAY)
	{
		QObjectList childs = rr->children();
		for (int i = 0; i < childs.count(); i++)
		{
			redis_reply *temp = qobject_cast<redis_reply *>(childs[i]);
			data << temp->string();
		}
	}
	else if (rr->type() == REDIS_RESULT_ERROR)
	{
		m_error = rr->error();
	}

	if (data.count() != 2 ) return QDateTime();
	return QDateTime::fromTime_t(data[0].toUInt());
}

void QRedis::readyRead()
{
  	do 
  	{
  		redis_reply *rr = get_redis_object(m_subssock);
  		if (rr->type() != REDIS_RESULT_ARRAY) return;
 
 		QObjectList childs = rr->children();
 		if (childs.count() != 3)  return;
 
 		redis_reply *temp = qobject_cast<redis_reply *>(childs[0]);
  
  		if (temp->string() == "message" || temp->string() == "pmessage")
  		{
 			redis_reply *temp1 = qobject_cast<redis_reply *>(childs[1]);
 			redis_reply *temp2 = qobject_cast<redis_reply *>(childs[2]);
 
  			emit subscribe(temp1->string(), temp2->string());
  		}
  	} while (m_subssock->bytesAvailable());	
}

QByteArray QRedis::format(const QList<QByteArray> &cmd)
{
    QByteArray result;
    result.append(QString("*%1\r\n").arg(cmd.length()));

    foreach(QByteArray part, cmd)
    {
        result.append("$");
        result.append(QString::number(part.size()));
        result.append("\r\n");
        result.append(part);
        result.append("\r\n");
    }

    return result;
}

redis_reply* QRedis::get_redis_object(QTcpSocket *sock)
{
	QByteArray ch = read(sock, 1);
	if (ch.isEmpty()) return 0;

	switch (ch.at(0))
	{
 	case '-':	// ERROR
 		return get_redis_error(sock);
 	case '+':	// STATUS
 		return get_redis_status(sock);
 	case ':':	// INTEGER
 		return get_redis_integer(sock);
 	case '$':	// STRING
 		return get_redis_string(sock);
 	case '*':	// ARRAY
 		return get_redis_array(sock);
	default:	// INVALID
		return 0;
	}
}

redis_reply* QRedis::get_redis_error(QTcpSocket *sock)
{
	QByteArray data = readLine(sock);
	if (data.isEmpty()) return 0;

	redis_reply* rr = new redis_reply;
	rr->setType(REDIS_RESULT_ERROR);
	rr->setData(data);

	return rr;
}

redis_reply* QRedis::get_redis_integer(QTcpSocket *sock)
{
	QByteArray data = readLine(sock);
	if (data.isEmpty()) return 0;

	redis_reply* rr = new redis_reply;
	rr->setType(REDIS_RESULT_INTEGER);
	rr->setData(data);

	return rr;
}

redis_reply* QRedis::get_redis_status(QTcpSocket *sock)
{
	QByteArray data = readLine(sock);
	if (data.isEmpty()) return 0;

	redis_reply* rr = new redis_reply;
	rr->setType(REDIS_RESULT_STATUS);
	rr->setData(data);

	return rr;
}

redis_reply* QRedis::get_redis_string(QTcpSocket *sock)
{
	QByteArray data = readLine(sock);
	if (data.isEmpty()) return 0;
	data.chop(2);
	int len = data.toInt();

	redis_reply* rr = new redis_reply;
	rr->setType(REDIS_RESULT_STRING);

	if (len == -1) 
	{
		rr->setData("nil");
	}
	else
	{
		QByteArray bulkdata = read(sock, len + 2);
		bulkdata.chop(2);
		rr->setData(QString::fromUtf8(bulkdata.data(), bulkdata.length()));
	}

	return rr;
}

redis_reply* QRedis::get_redis_array(QTcpSocket *sock)
{
	QByteArray data = readLine(sock);
	if (data.isEmpty()) return 0;
	data.chop(2);
	int count = data.toInt();

	redis_reply* rr = new redis_reply;
	rr->setType(REDIS_RESULT_ARRAY);

	if (count <= 0)	return rr;

	for (int i = 0; i < count; i++)
	{
		redis_reply* child = get_redis_object(sock);
		if (child == 0) return 0;
		child->setParent(rr);
	}

	return rr;
}

QByteArray QRedis::read(QTcpSocket *sock, int len)
{
	while (sock->bytesAvailable() < len) 
	{
		int ret = sock->waitForReadyRead(1000);
		if (!ret) return "";
	}

	return sock->read(len);
}

QByteArray QRedis::readLine(QTcpSocket *sock)
{
	if (sock->bytesAvailable() == 0)
	{
		bool ret = sock->waitForReadyRead(1000);
		if (!ret) return "";
	}
	return sock->readLine();
}

void QRedis::error(QAbstractSocket::SocketError)
{
	qWarning() << "The following error occurred: " << m_sock->errorString();
}

void QRedis::check()
{
	if (!m_isconnected)
	{
		m_sock->connectToHost(m_ip, m_port);
		m_subssock->connectToHost(m_ip, m_port);
	}
}

void QRedis::disconnected()
{
	m_sock->close();
	m_subssock->close();
	m_isconnected = false;
}

void QRedis::connected()
{
	m_isconnected = true;
	if (m_subssock != sender()) return;

	foreach (const QString &value, m_channels)
	{
		subscribe(value);
	}
	foreach (const QString &value, m_pchannels)
	{
		psubscribe(value);
	}
}