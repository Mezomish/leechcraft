/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "clientconnection.h"

#include <QTextCodec>
#include <plugininterface/util.h>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "ircprotocol.h"
#include "ircaccount.h"
#include "channelhandler.h"
#include "ircclient.h"
#include "channelclentry.h"
#include "ircmessage.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ClientConnection::ClientConnection (IrcAccount *account)
	: Account_ (account)
	, ProxyObject_ (0)
	, IsConnected_ (false)
	, FirstTimeConnect_ (true)
	{
		QObject *proxyObj = qobject_cast<IrcProtocol*> (account->
					GetParentProtocol ())->GetProxyObject ();
		ProxyObject_ = qobject_cast<IProxyObject*> (proxyObj);
	}

	ClientConnection::~ClientConnection ()
	{
		qDeleteAll (ChannelHandlers_);
	}
	
	QObject* ClientConnection::GetCLEntry (const QString& key, const QString& nick) const
	{
		if (ChannelHandlers_.contains (key))
			return ChannelHandlers_ [key]->GetParticipantEntry (nick).get ();
	}

	QList<QObject*> ClientConnection::GetCLEntries () const
	{
		QList<QObject*> result;
		Q_FOREACH (ChannelHandler *ch, ChannelHandlers_)
		{
			result << ch->GetCLEntry ();
			result << ch->GetParticipants ();
		}
		return result;
	}

	void ClientConnection::Sinchronize ()
	{

	}

	ChannelCLEntry* ClientConnection::JoinRoom (const ServerOptions& server, const ChannelOptions& channel)
	{
		QString id = QString ("%1@%2")
				.arg (channel.ChannelName_, channel.ServerName_);
		
		if (ChannelHandlers_.contains (id))
		{
			Entity e = Util::MakeNotification ("Azoth",
					tr ("This channel is already joined."),
					PCritical_);
			Core::Instance ().SendEntity (e);
			return 0;
		}

		ChannelHandler *ch = new ChannelHandler (server, channel, Account_);
		Core::Instance ().GetIrcClient ()->JoinChannel (server, channel, this);
		ChannelHandlers_ [id] = ch;
		Account_->ChangeState (EntryStatus (SOnline, QString ()));

		return ch->GetCLEntry ();
	}

	void ClientConnection::Unregister (ChannelHandler *ch)
	{
		ChannelHandlers_.remove (ch->GetChannelID ());
	}
	
	IrcMessage* ClientConnection::CreateMessage (IMessage::MessageType type,
			const QString& resource, const QString& body)
	{
// 		IrcMessage *msg = new IrcMessage (type,
// 				IMessage::DOut,
// 				chid,
// 				resource,
// 				this);
// 		msg->SetBody (body);
// 		msg->SetDateTime (QDateTime::currentDateTime ());

		return 0;
	}

	
	void ClientConnection::setChannelUseres (const QString& users, const QString& key)
	{
		Q_FOREACH (const QString& nick, users.split (' '))
			ChannelHandlers_ [key]->SetChannelUser (nick);
	}
	
	void ClientConnection::setSubject (const QString& subject, const QString& key)
	{
		ChannelHandlers_ [key]->SetSubject (subject);
	}

	void ClientConnection::handleMessageReceived (const QString& msg, const QString& id, const QString& nick)
	{
		if (ChannelHandlers_.contains (id))
		{
			QTextCodec *codec = QTextCodec::codecForName (ChannelHandlers_ [id]->
						GetServerOptions ().ServerEncoding_.toUtf8 ());
			QString mess =  codec->toUnicode (msg.toAscii ());
			ChannelHandlers_ [id]->HandleMessage (mess, nick);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "could not find source for";
	}

};
};
};