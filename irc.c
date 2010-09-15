/*
	quIRC - simple terminal-based IRC client
	Copyright (C) 2010 Edward Cree

	See quirc.c for license information
	irc: networking functions
*/

#include "irc.h"

int irc_connect(char *server, char *portno, fd_set *master, int *fdmax)
{
	int serverhandle;
	struct addrinfo hints, *servinfo;
	// Look up server
	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	int rv;
	if((rv = getaddrinfo(server, portno, &hints, &servinfo)) != 0)
	{
		char *err=(char *)gai_strerror(rv);
		w_buf_print(0, c_err, err, "getaddrinfo: ");
		return(0); // 0 indicates failure as rv is new serverhandle value
	}
	char sip[INET_ADDRSTRLEN];
	struct addrinfo *p;
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		inet_ntop(p->ai_family, &(((struct sockaddr_in*)p->ai_addr)->sin_addr), sip, sizeof(sip));
		if((serverhandle = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			w_buf_print(0, c_err, strerror(errno), "socket: ");
			continue;
		}
		if(fcntl(serverhandle, F_SETFD, O_NONBLOCK) == -1)
		{
			close(serverhandle);
			w_buf_print(0, c_err, strerror(errno), "fcntl: ");
			continue;
		}
		if(connect(serverhandle, p->ai_addr, p->ai_addrlen) == -1)
		{
			if(errno!=EINPROGRESS)
			{
				close(serverhandle);
				w_buf_print(0, c_err, strerror(errno), "connect: ");
				continue;
			}
		}
		break;
	}
	if (p == NULL)
	{
		w_buf_print(0, c_err, "failed to connect to server", "/connect: ");
		return(0); // 0 indicates failure as rv is new serverhandle value
	}
	freeaddrinfo(servinfo); // don't need this any more
	
	FD_SET(serverhandle, master);
	*fdmax=max(*fdmax, serverhandle);
	return(serverhandle);
}

int irc_conn_rest(int b, char *nick, char *username, char *fullname)
{
	bufs[b].live=true; // mark it as live
	if(bufs[b].autoent && bufs[b].autoent->nick)
		nick=bufs[b].autoent->nick;
	// TODO: Optionally send a PASS message before the NICK/USER
	char nickmsg[6+strlen(nick)];
	sprintf(nickmsg, "NICK %s", nick); // NICK <nickname>
	irc_tx(bufs[b].handle, nickmsg);
	struct utsname arch;
	uname(&arch);
	char usermsg[16+strlen(username)+strlen(arch.nodename)+strlen(fullname)];
	sprintf(usermsg, "USER %s 0 %s :%s", username, arch.nodename, fullname); // USER <user> <mode> <unused> <realname>
	irc_tx(bufs[b].handle, usermsg);
	return(0);
}

int autoconnect(fd_set *master, int *fdmax, servlist *serv)
{
	char cstr[36+strlen(serv->name)+strlen(serv->portno)];
	sprintf(cstr, "quIRC - connecting to %s", serv->name);
	settitle(cstr);
	sprintf(cstr, "Connecting to %s on port %s...", serv->name, serv->portno);
	setcolour(c_status);
	printf(CLA "\n");
	printf(LOCATE, height-2, 1);
	printf("%s" CLR "\n", cstr);
	resetcol();
	printf(CLA "\n");
	int serverhandle=irc_connect(serv->name, serv->portno, master, fdmax);
	if(serverhandle)
	{
		bufs=(buffer *)realloc(bufs, ++nbufs*sizeof(buffer));
		cbuf=nbufs-1;
		init_buffer(cbuf, SERVER, serv->name, buflines);
		bufs[cbuf].handle=serverhandle;
		bufs[cbuf].nick=strdup(serv->nick);
		bufs[cbuf].autoent=serv;
		if(serv) bufs[cbuf].ilist=serv->igns;
		bufs[cbuf].server=cbuf;
		add_to_buffer(cbuf, c_status, cstr);
		sprintf(cstr, "quIRC - connecting to %s", serv->name);
		settitle(cstr);
	}
	return(serverhandle);
}

int irc_tx(int fd, char * packet)
{
	//printf(">> %s\n\n", packet); // for debugging purposes
	char pq[512];
	low_quote(packet, pq);
	unsigned long l=min(strlen(pq), 511);
	unsigned long p=0;
	while(p<l)
	{
		signed long j=send(fd, pq+p, l-p, 0);
		if(j<1)
		{
			if(errno==EINTR)
				continue;
			return(p); // Something went wrong with send()!
		}
		p+=j;
	}
	send(fd, "\n", 1, 0);
	return(l); // Return the number of bytes sent
}

int irc_rx(int fd, char ** data)
{
	char buf[512];
	int l=0;
	bool cr=false;
	while(!cr)
	{
		long bytes=recv(fd, buf+l, 1, MSG_WAITALL);
		if(bytes>0)
		{
			char c=buf[l];
			if((strchr("\n\r", c)!=NULL) || (l>510))
			{
				cr=true;
				buf[l]=0;
			}
			l++;
		}
		else if(bytes<0)
		{
			if(errno==EINTR)
				continue;
			int b;
			for(b=0;b<nbufs;b++)
			{
				if((fd==bufs[b].handle) && (bufs[b].type==SERVER))
				{
					w_buf_print(b, c_err, strerror(errno), "irc_rx: recv:");
					bufs[b].live=false;
				}
			}
			cr=true; // just crash out with a partial message
			buf[l]=0;
		}
	}
	*data=strdup(buf);
	if(!*data)
		return(1);
	return(0);
}

message irc_breakdown(char *packet)
{
	char *pp=packet;
	message rv;
	if(*packet==':')
	{
		rv.prefix=strtok(packet, " ");
		rv.cmd=strtok(NULL, " ");
		packet=strtok(NULL, "");
	}
	else
	{
		rv.prefix=NULL;
		rv.cmd=strtok(packet, " ");
		packet=strtok(NULL, "");
	}
	if(rv.prefix) rv.prefix=low_dequote(rv.prefix);
	if(rv.cmd) rv.cmd=low_dequote(rv.cmd);
	if(!(rv.cmd&&packet))
	{
		rv.nargs=0;
		free(pp);
		return(rv);
	}
	int arg=0;
	char *p;
	while(*packet)
	{
		p=packet;
		if((*p==':')||(arg==15))
		{
			p++;
			rv.args[arg]=low_dequote(p);
			rv.nargs=15;
			break;
		}
		while(*p&&(*p!=' '))
		{
			p++;
		}
		rv.args[arg++]=low_dequote(packet);
		packet=p+(*p?1:0);
		*p=0;
	}
	rv.nargs=arg;
	free(pp);
	return(rv);
}

void message_free(message pkt)
{
	if(pkt.prefix) free(pkt.prefix);
	if(pkt.cmd) free(pkt.cmd);
	int arg;
	for(arg=0;arg<pkt.nargs;arg++)
		if(pkt.args[arg]) free(pkt.args[arg]);
}

void low_quote(char *from, char to[512])
{
	int o=0;
	while((*from) && (o<510))
	{
		char c=*from++;
		switch(c)
		{
			case '\n':
				to[o++]=MQUOTE;
				to[o++]='n';
			break;
			case '\r':
				to[o++]=MQUOTE;
				to[o++]='r';
			break;
			case MQUOTE:
				to[o++]=MQUOTE;
				to[o++]=MQUOTE;
			break;
			case '\\':
				if(*from=='0') // "\\0", is an encoded '\0'
				{
					to[o++]=MQUOTE; // because this will produce ^P 0, the proper representation
				}
				else
				{
					to[o++]=c;
				}
			break;
			case 0: // can't happen right now
				to[o++]=MQUOTE;
				to[o++]='0';
			break;
			default:
				to[o++]=c;
			break;
		}
	}
	to[o]=0;
}

char * low_dequote(char *buf)
{
	char *rv=(char *)malloc(512);
	if(!rv) return(NULL);
	char *p=buf;
	int o=0;
	while((*p) && (o<510))
	{
		if(*p==MQUOTE)
		{
			char c=*++p;
			switch(c)
			{
				case '0':
					rv[o++]='\\';
					rv[o]='0'; // We will have to defer '\0' handling as we can't stick '\0's in char *s (NUL terminated strings)
				break;
				case 'n':
					rv[o]='\n';
				break;
				case 'r':
					rv[o]='\r';
				break;
				case MQUOTE: // MQUOTE MQUOTE => MQUOTE, so fall through
				default:
					rv[o]=c;
				break;
			}
		}
		else
		{
			rv[o]=*p;
		}
		p++;o++;
	}
	rv[o]=0;
	return(rv);
}

char irc_to_upper(char c, cmap casemapping)
{
	// 97 to 126 -> 65 to 94 (CASEMAPPING=rfc1459; non-strict)
	// 97 to 125 -> 65 to 93 (CASEMAPPING=strict-rfc1459)
	// 97 to 122 -> 65 to 90 (CASEMAPPING=ascii)
	switch(casemapping)
	{
		case ASCII:
			if((97<=c)&&(c<=122))
				return(c-32);
		break;
		case STRICT_RFC1459:
			if((97<=c)&&(c<=125))
				return(c-32);
		break;
		case RFC1459: // fallthrough
		default:
			if((97<=c)&&(c<=126))
				return(c-32);
		break;
	}
	return(c);
}

char irc_to_lower(char c, cmap casemapping)
{
	// 65 to 94 -> 97 to 126 (CASEMAPPING=rfc1459; non-strict)
	// 65 to 93 -> 97 to 125 (CASEMAPPING=strict-rfc1459)
	// 65 to 90 -> 97 to 122 (CASEMAPPING=ascii)
	switch(casemapping)
	{
		case ASCII:
			if((65<=c)&&(c<=90))
				return(c+32);
		break;
		case STRICT_RFC1459:
			if((65<=c)&&(c<=93))
				return(c+32);
		break;
		case RFC1459: // fallthrough
		default:
			if((65<=c)&&(c<=94))
				return(c+32);
		break;
	}
	return(c);
}

int irc_strcasecmp(char *c1, char *c2, cmap casemapping)
{
	char t1,t2;
	while(*c1||*c2)
	{
		t1=irc_to_upper(*c1, casemapping);
		t2=irc_to_upper(*c2, casemapping);
		if(t2!=t1)
			return(t2>t1?-1:1);
		c1++;c2++;
	}
	return(0);
}

int irc_strncasecmp(char *c1, char *c2, int n, cmap casemapping)
{
	int i=0;
	char t1,t2;
	while((i<n)&&(c1[i]||c2[i]))
	{
		t1=irc_to_upper(c1[i], casemapping);
		t2=irc_to_upper(c2[i], casemapping);
		if(t2!=t1)
			return(t2>t1?-1:1);
		i++;
	}
	return(0);
}

int irc_numeric(message pkt, int b)
{
	int num=0;
	int b2;
	sscanf(pkt.cmd, "%d", &num);
	// arg[0] is invariably dest, with numeric replies; TODO check dest is us (not vital)
	int arg;
	switch(num)
	{
		case RPL_X_ISUPPORT:
			// 005 dest {[-]parameter|parameter=value}+ :are supported by this server
			for(arg=1;arg<pkt.nargs-1;arg++) // last argument is always the :postfix descriptive text
			{
				char *rest=pkt.args[arg];
				bool min=false;
				char *value=NULL;
				if(*rest=='-')
				{
					min=true;
					rest++;
				}
				else
				{
					char *eq=strchr(rest, '=');
					if(eq)
					{
						value=eq+1;
						*eq=0;
					}
				}
				if(strcmp(rest, "CASEMAPPING")==0)
				{
					if(value)
					{
						if(strcmp(value, "ascii")==0)
						{
							bufs[b].casemapping=ASCII;
						}
						else if(strcmp(value, "strict-rfc1459")==0)
						{
							bufs[b].casemapping=STRICT_RFC1459;
						}
						else
						{
							bufs[b].casemapping=RFC1459;
						}
					}
					else
					{
						bufs[b].casemapping=RFC1459;
					}
				}
				else
				{
					char isup[strlen(rest)+(value?strlen(value):0)+3];
					sprintf(isup, "%s%s%s%s", min?"-":"", rest, value?"=":"", value?value:"");
					w_buf_print(b, c_unn, isup, "RPL_ISUPPORT: ");
				}
			}
		break;
		case RPL_NAMREPLY:
			// 353 dest {=|/|\*|@} #chan :([@|\+]nick)+
			if(pkt.nargs<3)
			{
				e_buf_print(b, c_err, pkt, "RPL_NAMREPLY: Not enough arguments: ");
				break;
			}
			for(b2=0;b2<nbufs;b2++)
			{
				if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[2], bufs[b2].bname, bufs[b].casemapping)==0))
				{
					if(!bufs[b2].namreply)
					{
						bufs[b2].namreply=true;
						n_free(bufs[b2].nlist);
						bufs[b2].nlist=NULL;
					}
					int arg;
					for(arg=3;arg<pkt.nargs;arg++)
					{
						char *nn;
						while((nn=strtok(NULL, " ")))
						{
							if((*nn=='@')||(*nn=='+'))
								nn++;
							n_add(&bufs[b2].nlist, nn);
						}
					}
				}
			}
		break;
		case RPL_ENDOFNAMES:
			// 366 dest #chan :End of /NAMES list
			if(pkt.nargs<1)
			{
				e_buf_print(b, c_err, pkt, "RPL_ENDOFNAMES: Not enough arguments: ");
				break;
			}
			for(b2=0;b2<nbufs;b2++)
			{
				if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[1], bufs[b2].bname, bufs[b].casemapping)==0))
				{
					bufs[b2].namreply=false;
					char lmsg[32+strlen(pkt.args[1])];
					sprintf(lmsg, "NAMES list received for %s", pkt.args[1]);
					w_buf_print(b2, c_status, lmsg, "");
				}
			}
		break;
		case RPL_ENDOFMOTD: // 376 dest :End of MOTD command
		case RPL_MOTDSTART: // 375 dest :- <server> Message of the day -
		case RPL_MOTD: // 372 dest :- <text>
			if(pkt.nargs<2)
			{
				e_buf_print(b, c_err, pkt, num==RPL_MOTD?"RPL_MOTD: Not enough arguments: ":num==RPL_MOTDSTART?"RPL_MOTDSTART: Not enough arguments: ":num==RPL_ENDOFMOTD?"RPL_ENDOFMOTD: Not enough arguments: ":"RPL_MOTD???: Not enough arguments: ");
				break;
			}
			w_buf_print(b, c_notice[1], pkt.args[1], "");
		break;
		case ERR_NOMOTD: // 422 <dest> :MOTD File is missing
			if(pkt.nargs<2)
			{
				e_buf_print(b, c_err, pkt, "ERR_NOMOTD: Not enough arguments: ");
				break;
			}
			w_buf_print(b, c_notice[1], pkt.args[1], "");
		break;
		case RPL_TOPIC: // 332 dest <channel> :<topic>
			if(pkt.nargs<3)
			{
				e_buf_print(b, c_err, pkt, "RPL_TOPIC: Not enough arguments: ");
				break;
			}
			for(b2=0;b2<nbufs;b2++)
			{
				if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[1], bufs[b2].bname, bufs[b].casemapping)==0))
				{
					char tmsg[32+strlen(pkt.args[1])];
					sprintf(tmsg, "Topic for %s is ", pkt.args[1]);
					w_buf_print(b2, c_notice[1], pkt.args[2], tmsg);
					if(bufs[b2].topic) free(bufs[b2].topic);
					bufs[b2].topic=strdup(pkt.args[2]);
				}
			}
		break;
		case RPL_NOTOPIC: // 331 dest <channel> :No topic is set
			if(pkt.nargs<2)
			{
				e_buf_print(b, c_err, pkt, "RPL_NOTOPIC: Not enough arguments: ");
				break;
			}
			for(b2=0;b2<nbufs;b2++)
			{
				if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[1], bufs[b2].bname, bufs[b].casemapping)==0))
				{
					char tmsg[32+strlen(pkt.args[1])];
					sprintf(tmsg, "No topic is set for %s", pkt.args[1]);
					w_buf_print(b2, c_notice[1], tmsg, "");
					if(bufs[b2].topic) free(bufs[b2].topic);
				}
			}
		break;
		case RPL_X_TOPICWASSET: // 331 dest <channel> <nick> <time>
			if(pkt.nargs<3)
			{
				e_buf_print(b, c_err, pkt, "RPL_X_TOPICWASSET: Not enough arguments: ");
				break;
			}
			for(b2=0;b2<nbufs;b2++)
			{
				if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[1], bufs[b2].bname, bufs[b].casemapping)==0))
				{
					time_t when;
					sscanf(pkt.args[3], "%u", (unsigned int *)&when);
					char ts[256];
					struct tm *tm = gmtime(&when);
					size_t tslen = strftime(ts, sizeof(ts), "%H:%M:%S GMT on %a, %d %b %Y", tm); // TODO options controlling date format (eg. ISO 8601)
					char tmsg[32+strlen(pkt.args[2])+tslen];
					sprintf(tmsg, "Topic was set by %s at %s", pkt.args[2], ts);
					w_buf_print(b2, c_status, tmsg, "");
				}
			}
		break;
		case RPL_LUSERCLIENT: // 251 <dest> :There are <integer> users and <integer> invisible on <integer> servers"
		case RPL_LUSERME: // 255 <dest> ":I have <integer> clients and <integer> servers
			if(pkt.nargs<2)
			{
				e_buf_print(b, c_err, pkt, num==RPL_LUSERCLIENT?"RPL_LUSERCLIENT: Not enough arguments: ":num==RPL_LUSERME?"RPL_LUSERME: Not enough arguments: ":"RPL_LUSER???: Not enough arguments: ");
				break;
			}
			w_buf_print(b, c_status, pkt.args[1], ": ");
		break;
		case RPL_LUSEROP: // 252 <dest> <integer> :operator(s) online
		case RPL_LUSERUNKNOWN: // 253 <dest> <integer> :unknown connection(s)
		case RPL_LUSERCHANNELS: // 254 <dest> <integer> :channels formed
			if(pkt.nargs<3)
			{
				e_buf_print(b, c_err, pkt, num==RPL_LUSEROP?"RPL_LUSEROP: Not enough arguments: ":num==RPL_LUSERUNKNOWN?"RPL_LUSERUNKNOWN: Not enough arguments: ":num==RPL_LUSERCHANNELS?"RPL_LUSERCHANNELS: Not enough arguments: ":"RPL_LUSER???: Not enough arguments: ");
				break;
			}
			else
			{
				char lmsg[2+strlen(pkt.args[1])+strlen(pkt.args[2])];
				sprintf(lmsg, "%s %s", pkt.args[1], pkt.args[2]);
				w_buf_print(b, c_status, lmsg, ": ");
			}
		break;
		case RPL_X_LOCALUSERS: // 265 <dest> :Current Local Users: <integer>\tMax: <integer>
		case RPL_X_GLOBALUSERS: // 266 <dest> :Current Global Users: <integer>\tMax: <integer>
			if(pkt.nargs<2)
			{
				e_buf_print(b, c_err, pkt, num==RPL_X_LOCALUSERS?"RPL_X_LOCALUSERS: Not enough arguments: ":num==RPL_X_GLOBALUSERS?"RPL_X_GLOBALUSERS: Not enough arguments: ":"RPL_???USERS: Not enough arguments: ");
				break;
			}
			w_buf_print(b, c_status, pkt.args[1], ": ");
		break;
		default:
			e_buf_print(b, c_unn, pkt, "Unknown numeric: ");
		break;
	}
	return(num);
}

int rx_ping(message pkt, int b)
{
	// PING <sender>
	if(pkt.nargs<1)
	{
		e_buf_print(b, c_err, pkt, "Not enough arguments: ");
		return(0);
	}
	char pong[8+strlen(username)+strlen(pkt.args[0])];
	sprintf(pong, "PONG %s %s", username, pkt.args[0]); // PONG <user> <sender>
	return(irc_tx(bufs[b].server, pong));
}

int rx_mode(int b)
{
	// MODE <nick> ({\+|-}{i|w|o|O|r}*)*
	// We don't recognise modes yet, other than as a trigger for auto-join
	int fd=bufs[b].handle;
	servlist *serv=bufs[b].autoent;
	if(autojoin && serv->chans && !serv->join)
	{
		chanlist * curr=serv->chans;
		while(curr)
		{
			char joinmsg[8+strlen(curr->name)];
			sprintf(joinmsg, "JOIN %s %s", curr->name, curr->key?curr->key:"");
			irc_tx(fd, joinmsg);
			char jmsg[16+strlen(curr->name)];
			sprintf(jmsg, "auto-Joining %s", curr->name);
			w_buf_print(b, c_join[0], jmsg, "");
			curr=curr->next;
		}
		serv->join=true;
	}
	return(0);
}

int rx_kill(message pkt, int b, fd_set *master)
{
	// KILL <nick> <comment>
	if(pkt.nargs<1)
	{
		e_buf_print(b, c_err, pkt, "Not enough arguments: ");
		return(0);
	}
	int fd=bufs[b].handle;
	if(strcmp(pkt.args[0], bufs[b].nick)==0) // if it's us, we disconnect from the server
	{
		close(fd);
		FD_CLR(fd, master);
		int b2;
		for(b2=1;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) || (bufs[b2].server==0))
			{
				w_buf_print(b2, c_quit[0], pkt.args<2?"":pkt.args[1], "KILLed: ");
				bufs[b2].live=false;
			}
		}
		redraw_buffer();
	}
	else // if it's not us, generate quit messages into the relevant channel tabs
	{
		int b2;
		for(b2=1;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) || (bufs[b2].server==0))
			{
				if(n_cull(&bufs[b2].nlist, pkt.args[0]))
				{
					if(pkt.nargs<2)
					{
						char kmsg[24+strlen(pkt.args[0])+strlen(bufs[b].bname)];
						sprintf(kmsg, "=%s= has left %s (Killed)", pkt.args[0], bufs[b].bname);
						w_buf_print(b2, c_quit[1], kmsg, "");
					}
					else
					{
						char kmsg[28+strlen(pkt.args[0])+strlen(pkt.args[1])+strlen(bufs[b].bname)];
						sprintf(kmsg, "=%s= has left %s (Killed: %s)", pkt.args[0], bufs[b].bname, pkt.args[1]);
						w_buf_print(b2, c_quit[1], kmsg, "");
					}
				}
			}
		}
	}
	return(0);
}

int rx_kick(message pkt, int b)
{
	// KICK #chan user [comment]
	// From RFC2812: "The server MUST NOT send KICK messages with multiple channels or users to clients.  This is necessarily to maintain backward compatibility with old client software."
	if(pkt.nargs<2)
	{
		e_buf_print(b, c_err, pkt, "Not enough arguments: ");
		return(0);
	}
	if(irc_strcasecmp(pkt.args[1], bufs[b].nick, bufs[b].casemapping)==0) // if it's us, generate a message and de-live the channel
	{
		int b2;
		for(b2=1;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
			{
				w_buf_print(b2, c_quit[0], pkt.nargs<3?"(No reason)":pkt.args[2], "Kicked: ");
				bufs[b2].live=false;
			}
		}
		redraw_buffer();
	}
	else // if it's not us, just generate kick message
	{
		int b2;
		for(b2=1;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
			{
				if(n_cull(&bufs[b2].nlist, pkt.args[1]))
				{
					if(pkt.nargs<3)
					{
						char kmsg[32+strlen(pkt.args[1])];
						sprintf(kmsg, "=%s= was kicked.  (No reason)", pkt.args[1]);
						w_buf_print(b2, c_quit[1], kmsg, "");
					}
					else
					{
						char kmsg[32+strlen(pkt.args[1])+strlen(pkt.args[2])];
						sprintf(kmsg, "=%s= was kicked.  Reason: %s", pkt.args[1], pkt.args[2]);
						w_buf_print(b2, c_quit[1], kmsg, "");
					}
				}
			}
		}
	}
	return(0);
}

int rx_error(message pkt, int b, fd_set *master)
{
	// ERROR [message]
	// assume it's fatal
	int fd=bufs[b].handle;
	close(fd);
	FD_CLR(fd, master);
	int b2;
	for(b2=1;b2<nbufs;b2++)
	{
		if((bufs[b2].server==b) || (bufs[b2].server==0))
		{
			e_buf_print(b2, c_quit[0], pkt, "Disconnected: ");
			bufs[b2].live=false;
		}
	}
	return(redraw_buffer());
}

int rx_privmsg(message pkt, int b, bool notice)
{
	// :nick!user@server PRIVMSG msgtarget text
	// :nick!user@server NOTICE msgtarget text
	if(pkt.nargs<2)
	{
		e_buf_print(b, c_err, pkt, "Not enough arguments: ");
		return(0);
	}
	char *src=pkt.prefix?pkt.prefix:""
	char *bang=strchr(src, '!');
	if(bang)
		*bang++=0;
	else
		bang="";
	char *host=strchr(bang, '@');
	if(host)
		*host++=0;
	else
		host="";
	char nm[strlen(src)+strlen(bang)+strlen(host)+3];
	char *from=strdup(src);
	crush(&from, maxnlen);
	int b2;
	bool match=false;
	for(b2=0;b2<nbufs;b2++)
	{
		if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
		{
			match=true;
			sprintf(nm, "%s@%s", src, host);
			if(i_match(bufs[b].ilist, nm, false, bufs[b].casemapping)||i_match(bufs[0].ilist, nm, false, bufs[b].casemapping))
				break;
			if(i_match(bufs[b2].ilist, nm, false, bufs[b].casemapping))
				continue;
			sprintf(nm, "%s@%s", bang, host);
			if(i_match(bufs[b].ilist, nm, false, bufs[b].casemapping)||i_match(bufs[0].ilist, nm, false, bufs[b].casemapping))
				break;
			if(i_match(bufs[b2].ilist, nm, false, bufs[b].casemapping))
				continue;
			sprintf(nm, "%s", src);
			if(i_match(bufs[b].ilist, nm, false, bufs[b].casemapping)||i_match(bufs[0].ilist, nm, false, bufs[b].casemapping))
				break;
			if(i_match(bufs[b2].ilist, nm, false, bufs[b].casemapping))
				continue;
			if(*pkt.args[1]==1) // CTCP
			{
				ctcp(pkt.args[1], from, src, b2);
			}
			else
			{
				char tag[maxnlen+4]; // TODO this tag-making bit ought to be refactored really
				memset(tag, ' ', maxnlen+3);
				sprintf(tag+maxnlen-strlen(from), "<%s> ", from);
				w_buf_print(b2, notice?c_notice[1]:c_msg[1], pkt.args[1], tag);
			}
		}
	}
	if(!match)
	{
		sprintf(nm, "%s@%s", src, host);
		if(i_match(bufs[b].ilist, nm, true, bufs[b].casemapping)||i_match(bufs[0].ilist, nm, true, bufs[b].casemapping))
			return(0);
		sprintf(nm, "%s@%s", bang, host);
		if(i_match(bufs[b].ilist, nm, true, bufs[b].casemapping)||i_match(bufs[0].ilist, nm, true, bufs[b].casemapping))
			return(0);
		sprintf(nm, "%s", src);
		if(i_match(bufs[b].ilist, nm, true, bufs[b].casemapping)||i_match(bufs[0].ilist, nm, true, bufs[b].casemapping))
			return(0);
		if(irc_strcasecmp(pkt.args[0], bufs[b].nick, bufs[b].casemapping)==0)
		{
			if(*msg==1) // CTCP
			{
				ctcp(pkt.args[1], from, src, b);
			}
			else
			{
				char tag[maxnlen+9];
				memset(tag, ' ', maxnlen+8);
				sprintf(tag+maxnlen-strlen(from), "(from %s) ", from);
				w_buf_print(b, notice?c_notice[1]:c_msg[1], pkt.args[1], tag);
			}
		}
		else
		{
			e_buf_print(b, c_err, pkt, "Bad destination: ");
		}
	}
	free(from);
	return(0);
}

int rx_topic(message pkt, int b)
{
	// TOPIC <dest> [<topic>]
	if(pkt.nargs<1)
	{
		e_buf_print(b, c_err, pkt, "Not enough arguments: ");
		return(0);
	}
	char *src=pkt.prefix?pkt.prefix:"";
	char *bang=strchr(src, '!');
	if(bang)
		*bang=0;
	char *from=strdup(src);
	scrush(&from, maxnlen);
	if(pkt.nargs<2)
	{
		char tag[maxnlen+20];
		sprintf(tag, "%s removed the Topic", from);
		int b2;
		bool match=false;
		for(b2=0;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
			{
				w_buf_print(b2, c_notice[1], "", tag);
				match=true;
				if(bufs[b2].topic) free(bufs[b2].topic);
				bufs[b2].topic=NULL;
			}
		}
	}
	else
	{
		char tag[maxnlen+20];
		sprintf(tag, "%s set the Topic to ", from);
		int b2;
		bool match=false;
		for(b2=0;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(dest, bufs[b2].bname, bufs[b].casemapping)==0))
			{
				w_buf_print(b2, c_notice[1], pkt.args[1], tag);
				match=true;
				if(bufs[b2].topic) free(bufs[b2].topic);
				bufs[b2].topic=strdup(msg);
			}
		}
	}
	free(from);
	return(match?0:1);
}

int rx_join(message pkt, int b)
{
	// :nick!user@server JOIN #chan
	if(pkt.nargs<1)
	{
		e_buf_print(b, c_err, pkt, "Not enough arguments: ");
		return(0);
	}
	char *src=pkt.prefix?pkt.prefix:""
	char *bang=strchr(src, '!');
	if(bang)
		*bang++=0;
	if(strcmp(src, bufs[b].nick)==0)
	{
		char dstr[20+strlen(src)+strlen(pkt.args[0])];
		sprintf(dstr, "You (%s) have joined %s", src, pkt.args[0]);
		char cstr[16+strlen(src)+strlen(pkt.args[0])];
		sprintf(cstr, "quIRC - %s on %s", src, pkt.args[0]);
		settitle(cstr);
		int b2;
		for(b2=1;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
			{
				cbuf=b2;
				break;
			}
		}
		if(b2>=nbufs)
		{
			bufs=(buffer *)realloc(bufs, ++nbufs*sizeof(buffer));
			init_buffer(nbufs-1, CHANNEL, dest+1, buflines);
			cbuf=nbufs-1;
			bufs[cbuf].server=bufs[b].server;
		}
		w_buf_print(cbuf, c_join[0], dstr, "");
		bufs[cbuf].handle=bufs[bufs[cbuf].server].handle;
		bufs[cbuf].live=true;
	}
	else
	{
		int b2;
		bool match=false;
		for(b2=0;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
			{
				match=true;
				char dstr[16+strlen(src)+strlen(pkt.args[0])];
				sprintf(dstr, "=%s= has joined %s", src, pkt.args[0]);
				w_buf_print(b2, c_join[1], dstr, "");
				n_add(&bufs[b2].nlist, src);
			}
		}
		if(!match)
		{
			e_buf_print(b, c_err, pkt, "Bad destination: ");
		}
	}
	return(0);
}

int rx_part(message pkt, int b)
{
	// :nick!user@server PART #chan message
	if(pkt.nargs<1)
	{
		e_buf_print(b, c_err, pkt, "Not enough arguments: ");
		return(0);
	}
	char *src=pkt.prefix?pkt.prefix:""
	char *bang=strchr(src, '!');
	if(bang)
		*bang++=0;
	if(strcmp(src, bufs[b].nick)==0)
	{
		int b2;
		for(b2=0;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
			{
				if(b2==cbuf)
				{
					cbuf=b;
					char cstr[24+strlen(bufs[b].bname)];
					sprintf(cstr, "quIRC - connected to %s", bufs[b].bname);
					settitle(cstr);
				}
				bufs[b2].live=false;
				free_buffer(b2);
			}
		}
	}
	else
	{
		int b2;
		bool match=false;
		for(b2=0;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL) && (irc_strcasecmp(pkt.args[0], bufs[b2].bname, bufs[b].casemapping)==0))
			{
				match=true;
				if(pkt.nargs<2)
				{
					char dstr[16+strlen(src)+strlen(pkt.args[0])];
					sprintf(dstr, "=%s= has left %s", src, pkt.args[0]);
					w_buf_print(b2, c_part[1], dstr, "");
				}
				else
				{
					char dstr[24+strlen(src)+strlen(pkt.args[0])+strlen(pkt.args[1])];
					sprintf(dstr, "=%s= has left %s (Part: %s)", src, pkt.args[0], pkt.args[1]);
					w_buf_print(b2, c_part[1], dstr, "");
				}
				n_cull(&bufs[b2].nlist, src);
			}
		}
		if(!match)
		{
			e_buf_print(b, c_err, pkt, "Bad destination: ");
		}
	}
	return(0);
}

int rx_quit(message pkt, int b)
{
	// :nick!user@server QUIT message
	char *src=pkt.prefix?pkt.prefix:""
	char *bang=strchr(src, '!');
	if(bang)
		*bang++=0;
	char *reason=pkt.nargs>0?pkt.args[0]:"";
	if(strcmp(src, bufs[b].nick)==0) // this shouldn't happen
	{
		e_buf_print(b, c_err, pkt, "Should not be from us: ");
	}
	else
	{
		int b2;
		for(b2=0;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL))
			{
				if(n_cull(&bufs[b2].nlist, src))
				{
					char dstr[24+strlen(src)+strlen(bufs[b].bname)+strlen(reason)];
					sprintf(dstr, "=%s= has left %s (%s)", src, bufs[b].bname, reason);
					w_buf_print(b2, c_quit[1], dstr, "");
				}
			}
		}
	}
	return(0);
}

int rx_nick(int b, char *packet, char *pdata)
{
	char *dest=strtok(NULL, " \t");
	char *src=packet+1;
	char *bang=strchr(src, '!');
	if(bang)
		*bang=0;
	if(!isalpha(*src))
		src++;
	if(strcmp(dest+1, bufs[b].nick)==0)
	{
		char dstr[30+strlen(src)+strlen(dest+1)];
		sprintf(dstr, "You (%s) are now known as %s", src, dest+1);
		int b2;
		for(b2=0;b2<nbufs;b2++)
		{
			if(bufs[b2].server==b)
			{
				w_buf_print(b2, c_nick[1], dstr, "");
				n_cull(&bufs[b2].nlist, src);
				n_add(&bufs[b2].nlist, dest+1);
			}
		}
	}
	else
	{
		int b2;
		bool match=false;
		for(b2=0;b2<nbufs;b2++)
		{
			if((bufs[b2].server==b) && (bufs[b2].type==CHANNEL))
			{
				match=true;
				if(n_cull(&bufs[b2].nlist, src))
				{
					n_add(&bufs[b2].nlist, dest+1);
					char dstr[30+strlen(src)+strlen(dest+1)];
					sprintf(dstr, "=%s= is now known as %s", src, dest+1);
					w_buf_print(b2, c_nick[1], dstr, "");
				}
			}
		}
		if(!match)
		{
			w_buf_print(b, c_err, pdata, "?? ");
		}
	}
	return(0);
}

int ctcp(char *msg, char *from, char *src, int b2)
{
	int fd=bufs[b2].handle;
	if(strncmp(msg, "\001ACTION ", 8)==0)
	{
		msg[strlen(msg)-1]=0; // remove trailing \001
		char tag[maxnlen+4];
		memset(tag, ' ', maxnlen+3);
		sprintf(tag+maxnlen+2-strlen(from), "%s ", from);
		w_buf_print(b2, c_actn[1], msg+8, tag);
	}
	else if(strncmp(msg, "\001FINGER", 7)==0)
	{
		char resp[32+strlen(src)+strlen(fname)];
		sprintf(resp, "NOTICE %s \001FINGER :%s\001", src, fname);
		irc_tx(fd, resp);
	}
	else if(strncmp(msg, "\001PING", 5)==0)
	{
		char resp[16+strlen(src)+strlen(msg)];
		sprintf(resp, "NOTICE %s %s", src, msg);
		irc_tx(fd, resp);
	}
	else if(strncmp(msg, "\001CLIENTINFO", 11)==0)
	{
		char resp[64+strlen(src)];
		sprintf(resp, "NOTICE %s \001CLIENTINFO ACTION FINGER PING CLIENTINFO VERSION\001", src);
		irc_tx(fd, resp);
	}
	else if(strncmp(msg, "\001VERSION", 8)==0)
	{
		char resp[32+strlen(src)+strlen(version)+strlen(CC_VERSION)];
		sprintf(resp, "NOTICE %s \001VERSION %s:%s:%s\001", src, "quIRC", version, CC_VERSION);
		irc_tx(fd, resp);
	}
	else
	{
		char tag[maxnlen+9];
		memset(tag, ' ', maxnlen+8);
		sprintf(tag+maxnlen-strlen(from), "(from %s) ", from);
		char *cmd=msg+1;
		char *space=strchr(cmd, ' ');
		if(space)
			*space=0;
		char cmsg[32+strlen(cmd)];
		sprintf(cmsg, "Unrecognised CTCP %s (ignoring)", cmd);
		w_buf_print(b2, c_unk, cmsg, tag);
		char resp[32+strlen(src)+strlen(cmd)];
		sprintf(resp, "NOTICE %s \001ERRMSG %s\001", src, cmd);
		irc_tx(fd, resp);
	}
	return(0);
}
