#ifndef _QREDIS_H_
#define _QREDIS_H_

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include <QDateTime>

typedef enum
{
	REDIS_RESULT_UNKOWN,
	REDIS_RESULT_NIL,
	REDIS_RESULT_ERROR,
	REDIS_RESULT_STATUS,
	REDIS_RESULT_INTEGER,
	REDIS_RESULT_STRING,
	REDIS_RESULT_ARRAY,
} redis_reply_t;

class redis_reply : public QObject
{
	Q_OBJECT
public:
	redis_reply(QObject * parent = 0);
	redis_reply_t type();
	qlonglong integer();
	QString status();
	QString error();
	QString string();
	void setData(const QString &data);
	void setType(redis_reply_t type);
private:
	redis_reply_t reply_type_;
	QString str;
};

class QRedis : public QObject
{
	Q_OBJECT
public:
	QRedis(QObject * parent = 0);
	~QRedis();
public:
	void connectHost(const QString &host, const quint16 port = 6379);
public:
	///////////////////////key//////////////////////////////
	int del(const QString &key);
	int del(const QStringList &keys);
	bool exists(const QString &key);
	bool expire(const QString &key, qlonglong secs);
	bool expireat(const QString &key, qlonglong timestamp);
	QStringList keys(const QString &pattern);
	bool move(const QString &key, int db);
	bool persist(const QString &key);
	bool pexpire(const QString &key, qlonglong mils);
	bool pexpireat(const QString &key, qlonglong milstimestamp);
	qlonglong pttl(const QString &key);
	QString randomkey();
	bool rename(const QString &key, const QString &newkey);
	bool renamenx(const QString &key, const QString &newkey);
	qlonglong ttl(const QString &key);
	QString type(const QString &key);
	///////////////////////string//////////////////////////////
	int append(const QString &key, const QString &value);
	qlonglong decr(const QString &key);
	qlonglong decrby(const QString &key, qlonglong value);
	QString get(const QString &key);
	QString getrange(const QString &key, qlonglong start, qlonglong stop);
	QString getset(const QString &key, const QString &value);
	qlonglong incr(const QString &key);
	qlonglong incrby(const QString &key, qlonglong value);
	qreal incrbyfloat(const QString &key, qreal value);
	QStringList mget(const QStringList &keys);
	void mset(const QStringList &keyvalues);
	bool msetnx(const QStringList &keyvalues);
	bool psetex(const QString &key, qlonglong mils, const QString &value);
	bool set(const QString &key, const QString &value);
	bool setex(const QString &key, qlonglong secs, const QString &value);
	bool setnx(const QString &key, const QString &value);
	qlonglong setrange(const QString &key, qlonglong offset, const QString &value);
	qlonglong strlen(const QString &key);
	///////////////////////hash//////////////////////////////
	qlonglong hdel(const QString &key, const QString &field);
	qlonglong hdel(const QString &key, const QStringList &fields);
	bool hexists(const QString &key);
	QString hget(const QString &key, const QString &field);
	QStringList hgetall(const QString &key);
	qlonglong hincrby(const QString &key, const QString &field, qlonglong value);
	qreal hincrbyfloat(const QString &key, const QString &field, qreal value);
	QStringList hkeys(const QString &key);
	qlonglong hlen(const QString &key);
	QStringList hmget(const QString &key, const QStringList &fields);
	bool hmset(const QString &key, const QStringList &fvs);
	int hset(const QString &key, const QString &field, const QString &value);
	bool hsetnx(const QString &key, const QString &field, const QString &value);
	QStringList hvals(const QString &key);
	///////////////////////list//////////////////////////////
	QString lindex(const QString &key, qlonglong index);
	qlonglong llen(const QString &key);
	QString lpop(const QString &key);
	qlonglong lpush(const QString &key, const QString &value);
	qlonglong lpush(const QString &key, const QStringList &values);
	QStringList lrange(const QString &key, qlonglong start, qlonglong stop);
	qlonglong lrem(const QString &key, int count, const QString &value);
	bool lset(const QString &key, int index, const QString &value);
	QString rpop(const QString &key);
	qlonglong rpush(const QString &key, const QString &value);
	qlonglong rpush(const QString &key, const QStringList &values);
	///////////////////////set//////////////////////////////
	qlonglong sadd(const QString &key, const QString &value);
	qlonglong sadd(const QString &key, const QStringList &values);
	qlonglong scard(const QString &key);
	QStringList sdiff(const QStringList &keys);
	QStringList sinter(const QStringList &keys);
	bool sismember(const QString &key, const QString &value);
	QStringList smembers(const QString &key);
	qlonglong srem(const QString &key, const QString &value);
	qlonglong srem(const QString &key, const QStringList &values);
	QStringList sunion(const QStringList &keys);
	///////////////////////pub/sub//////////////////////////////
	void psubscribe(const QString &pattern);
	void psubscribe(const QStringList &patterns);
	int publish(const QString &channel, const QString &data);
	void punsubscribe();
	void punsubscribe(const QString &pattern);
	void punsubscribe(const QStringList &patterns);
	void subscribe(const QString &channel);
	void subscribe(const QStringList &channels);
	void unsubscribe();
	void unsubscribe(const QString &channel);
	void unsubscribe(const QStringList &channels);
	///////////////////////script//////////////////////////////
	QStringList eval(const QString &script, const QStringList &args);
	QStringList evalsha(const QString &sha1, const QStringList &args);
	bool scriptexists(const QString &sha1);
	QStringList scriptexists(const QStringList &sha1s);
	void scriptflush();
	void scriptkill();
	QString scriptload(const QString &script);
	///////////////////////connection//////////////////////////////
	bool auth(const QString &pw);
	bool ping();
	void quit();
	bool select(int db);
	///////////////////////server//////////////////////////////
	bool bgsave();
	QString clientgetname();
	bool clientkill(const QString &ipport);
	QStringList clientlist();
	bool clientsetname(const QString &name);
	qlonglong dbsize();
	void flushall();
	void flushdb();
	QString info();
	QDateTime time();
	///////////////////////other//////////////////////////////
	QString lastError() { return m_error; }
signals:
	void subscribe(const QString &channel, const QString &data);
private slots:
	void check();
	void disconnected();
	void connected();
	void readyRead();
	void error(QAbstractSocket::SocketError);
protected:
	QByteArray format(const QList<QByteArray> &cmd);
	QByteArray read(QTcpSocket *sock, int len);
	QByteArray readLine(QTcpSocket *sock);
	redis_reply* get_redis_object(QTcpSocket *sock);
	redis_reply* get_redis_error(QTcpSocket *sock);
	redis_reply* get_redis_status(QTcpSocket *sock);
	redis_reply* get_redis_integer(QTcpSocket *sock);
	redis_reply* get_redis_string(QTcpSocket *sock);
	redis_reply* get_redis_array(QTcpSocket *sock);
protected:
	QTcpSocket *m_sock;
	QTcpSocket *m_subssock;
	bool m_isconnected;
	int m_port;
	QString m_ip;
	QString m_error;
	QSet<QString> m_channels, m_pchannels;
};

#endif //_QREDIS_H_
