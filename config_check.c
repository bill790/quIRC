/* Generated by genconfig */
int conf_check(void)
{
	if(width<30)
	{
		asb_failsafe(c_status, "width set to minimum 30");
		width=30;
	}
	if(height<5)
	{
		asb_failsafe(c_status, "height set to minimum 5");
		height=5;
	}
	if(mirc_colour_compat>2)
	{
		asb_failsafe(c_status, "mcc set to maximum 2");
		mirc_colour_compat=2;
	}
	if(force_redraw>3)
	{
		asb_failsafe(c_status, "fred set to maximum 3");
		force_redraw=3;
	}
	if(buflines<32)
	{
		asb_failsafe(c_status, "buf set to minimum 32");
		buflines=32;
	}
	if(maxnlen<4)
	{
		asb_failsafe(c_status, "mnln set to minimum 4");
		maxnlen=4;
	}
	if(tping<15)
	{
		asb_failsafe(c_status, "tping set to minimum 15");
		tping=15;
	}
	if(ts>4)
	{
		asb_failsafe(c_status, "ts set to maximum 4");
		ts=4;
	}
	return(0);
}
