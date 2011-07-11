/* Generated by genconfig */
				if(strcmp(opt, "width")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						width=value;
					}
					else
						width=80;
					if(width<30)
						width=30;
					if(!quiet)
					{
						char smsg[37];
						sprintf(smsg, "display width set to %u", width);
						add_to_buffer(cbuf, c_status, smsg, "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "height")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						height=value;
					}
					else
						height=24;
					if(height<5)
						height=5;
					if(!quiet)
					{
						char smsg[38];
						sprintf(smsg, "display height set to %u", height);
						add_to_buffer(cbuf, c_status, smsg, "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "mcc")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						mirc_colour_compat=value;
					}
					else
						mirc_colour_compat=1;
					if(mirc_colour_compat>2)
						mirc_colour_compat=2;
					if(!quiet)
					{
						if(mirc_colour_compat)
						{
							char lmsg[57];
							sprintf(lmsg, "mirc colour compatibility level %u enabled", mirc_colour_compat);
							add_to_buffer(cbuf, c_status, lmsg, "/set: ");
						}
						else
							add_to_buffer(cbuf, c_status, "mirc colour compatibility disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-mcc")==0)
				{
					mirc_colour_compat=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "mirc colour compatibility disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "fred")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						force_redraw=value;
					}
					else
						force_redraw=0;
					if(force_redraw>3)
						force_redraw=3;
					if(!quiet)
					{
						if(force_redraw)
						{
							char lmsg[44];
							sprintf(lmsg, "force-redraw level %u enabled", force_redraw);
							add_to_buffer(cbuf, c_status, lmsg, "/set: ");
						}
						else
							add_to_buffer(cbuf, c_status, "force-redraw disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-fred")==0)
				{
					force_redraw=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "force-redraw disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "buf")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						buflines=value;
					}
					else
						buflines=256;
					if(buflines<32)
						buflines=32;
					if(!quiet)
					{
						char smsg[36];
						sprintf(smsg, "buffer lines set to %u", buflines);
						add_to_buffer(cbuf, c_status, smsg, "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "mnln")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						maxnlen=value;
					}
					else
						maxnlen=16;
					if(maxnlen<4)
						maxnlen=4;
					if(!quiet)
					{
						char smsg[39];
						sprintf(smsg, "max nick length set to %u", maxnlen);
						add_to_buffer(cbuf, c_status, smsg, "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "fwc")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							full_width_colour=value;
						}
						else if(strcmp(val, "+")==0)
						{
							full_width_colour=true;
						}
						else if(strcmp(val, "-")==0)
						{
							full_width_colour=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'fwc' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						full_width_colour=true;
					if(!quiet)
					{
						if(full_width_colour)
							add_to_buffer(cbuf, c_status, "full width colour enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "full width colour disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-fwc")==0)
				{
					full_width_colour=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "full width colour disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "hts")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							hilite_tabstrip=value;
						}
						else if(strcmp(val, "+")==0)
						{
							hilite_tabstrip=true;
						}
						else if(strcmp(val, "-")==0)
						{
							hilite_tabstrip=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'hts' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						hilite_tabstrip=true;
					if(!quiet)
					{
						if(hilite_tabstrip)
							add_to_buffer(cbuf, c_status, "highlight tabstrip enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "highlight tabstrip disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-hts")==0)
				{
					hilite_tabstrip=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "highlight tabstrip disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "tsb")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							tsb=value;
						}
						else if(strcmp(val, "+")==0)
						{
							tsb=true;
						}
						else if(strcmp(val, "-")==0)
						{
							tsb=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'tsb' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						tsb=true;
					if(!quiet)
					{
						if(tsb)
							add_to_buffer(cbuf, c_status, "top status bar enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "top status bar disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-tsb")==0)
				{
					tsb=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "top status bar disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "tping")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						tping=value;
					}
					else
						tping=30;
					if(tping<15)
						tping=15;
					if(!quiet)
					{
						char smsg[42];
						sprintf(smsg, "outbound ping time set to %u", tping);
						add_to_buffer(cbuf, c_status, smsg, "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "ts")==0)
				{
					if(val)
					{
						unsigned int value;
						sscanf(val, "%u", &value);
						ts=value;
					}
					else
						ts=1;
					if(ts>4)
						ts=4;
					if(!quiet)
					{
						if(ts)
						{
							char lmsg[44];
							sprintf(lmsg, "timestamping level %u enabled", ts);
							add_to_buffer(cbuf, c_status, lmsg, "/set: ");
						}
						else
							add_to_buffer(cbuf, c_status, "timestamping disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-ts")==0)
				{
					ts=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "timestamping disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "utc")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							utc=value;
						}
						else if(strcmp(val, "+")==0)
						{
							utc=true;
						}
						else if(strcmp(val, "-")==0)
						{
							utc=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'utc' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						utc=true;
					if(!quiet)
					{
						if(utc)
							add_to_buffer(cbuf, c_status, "UTC timestamps enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "UTC timestamps disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-utc")==0)
				{
					utc=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "UTC timestamps disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "its")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							its=value;
						}
						else if(strcmp(val, "+")==0)
						{
							its=true;
						}
						else if(strcmp(val, "-")==0)
						{
							its=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'its' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						its=true;
					if(!quiet)
					{
						if(its)
							add_to_buffer(cbuf, c_status, "input clock enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "input clock disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-its")==0)
				{
					its=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "input clock disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "quiet")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							quiet=value;
						}
						else if(strcmp(val, "+")==0)
						{
							quiet=true;
						}
						else if(strcmp(val, "-")==0)
						{
							quiet=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'quiet' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						quiet=true;
					if(!quiet)
					{
						if(quiet)
							add_to_buffer(cbuf, c_status, "quiet mode enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "quiet mode disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-quiet")==0)
				{
					quiet=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "quiet mode disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "debug")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							debug=value;
						}
						else if(strcmp(val, "+")==0)
						{
							debug=true;
						}
						else if(strcmp(val, "-")==0)
						{
							debug=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'debug' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						debug=true;
					if(!quiet)
					{
						if(debug)
							add_to_buffer(cbuf, c_status, "debugging enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "debugging disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-debug")==0)
				{
					debug=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "debugging disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "titles")==0)
				{
					if(val)
					{
						if(isdigit(*val))
						{
							unsigned int value;
							sscanf(val, "%u", &value);
							titles=value;
						}
						else if(strcmp(val, "+")==0)
						{
							titles=true;
						}
						else if(strcmp(val, "-")==0)
						{
							titles=false;
						}
						else
						{
							add_to_buffer(cbuf, c_err, "option 'titles' is boolean, use only 0/1 or -/+ to set", "/set: ");
						}
					}
					else
						titles=true;
					if(!quiet)
					{
						if(titles)
							add_to_buffer(cbuf, c_status, "xterm title enabled", "/set: ");
						else
							add_to_buffer(cbuf, c_status, "xterm title disabled", "/set: ");
					}
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
				else if(strcmp(opt, "no-titles")==0)
				{
					titles=0;
					if(!quiet) add_to_buffer(cbuf, c_status, "xterm title disabled", "/set: ");
					int buf;
					for(buf=0;buf<nbufs;buf++)
						bufs[buf].dirty=true;
					redraw_buffer();
				}
