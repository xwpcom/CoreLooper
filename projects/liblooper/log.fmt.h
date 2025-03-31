#pragma once

/*
//对int		"%xd"
//对double	"%.xf"
vt0(int)
vt0(double)

vt1(int)
vt1(double)
vt2
vt3
vt4
vt5
vt6
*/
static char buf[32] = {};
const char* vt4(int v)
{
	_snprintf(buf, sizeof(buf),"%04d", v);
	return buf;
}

const char* vt4(double v)
{
	_snprintf(buf, sizeof(buf), "%.4f", v);
	return buf;
}
