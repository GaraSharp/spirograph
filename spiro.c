/*
 * title : spiro.c  -  スピログラフ研究
 * begin : 2025-09-06 07:07:26 
 * base  : langton.c
 * note  : install timer callback
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <math.h>

//  Windows size
#define  WIDTH    640
#define  HEIGHT   640
#define  DIVIDE   (360*4)

// wait value [msec]
#define  WAITING  2

//  grobal var.

//  screen points

int     xp = WIDTH/2, yp = HEIGHT/2;

int     a   =  100 ;   // 長径
int     b   =  100 ;   // 短径
int     d   =   70 ;   // 小円半径
int     dd  =   70 ;   // 小円中の「ペン穴」の半径位置

double  th  = 0 ;   //  楕円中心からの小円の接触位置角 (deg)
double  dt  = 2*M_PI/DIVIDE ;
double  e   =    0 ;

double   x  = 0 ;
double   y  = 0 ;
double   xi = 0 ;
double   yi = 0 ;
double   xx = 0 ;
double   yy = 0 ;
double   xd = 0 ;
double   yd = 0 ;
double   xt = 0 ;
double   yt = 0 ;
double   ph = 0 ;
double   rh = 0 ;


double  arc = 0 ;   //  0..th の楕円周
double  y_0 = 0;  //
double  y_1 = 0;  //



XtAppContext app_con;
Widget toplevel;
Arg     arg_list[5];
GC gc;
XGCValues value;
Display *disp;
Window win;


//  prototype reference
void quit_proc();

//  Resources ; Push Mouse button 1 for exit
String fallback_resources[] = {
    ".Spiro.translations: #override\\n\
     <Btn1Up>: Quit()",
    // ".Spiro.geometry: 640x640",
    NULL,
};

XtActionsRec actions[] = {
    {"Quit", (XtActionProc)quit_proc},
};

//  fonctions

//  台形積分
double  daikei(double y_0, double y_1, double dt) {
    return (y_0+y_1)*dt/2 ;
//    return (y_1)*dt ;
}


//  first initalize graphics
void
drawNextPt(Widget w, caddr_t *client_data, caddr_t *call_data)
{
    double t = 2*M_PI*th/DIVIDE ;
    //
    xx = WIDTH /2+a*cos(t) ;
    yy = HEIGHT/2-b*sin(t) ;

    ph = atan2( a/b*sin(t), cos(t) ) ;
    // 
    xd = xx+d*cos(ph) ;
    yd = yy-d*sin(ph) ;

    th  += dt ;
    y_1 = a*sqrt( 1-e*e*sin(th)*sin(th) ) ;
    arc += daikei(y_0, y_1, dt) ;
    y_0 = y_1 ;

    rh = arc/d + ph ;
    //
    xt = xd+dd*cos(rh) ;
    yt = yd-dd*sin(rh) ;

    //canvas.move(ball_id, xd-x, yd-y)
    x = xd ; 
    y = yd ;

    //canvas.move(indx_id, xt-xi, yt-yi)
    //set_pixel(int(xt),int(yt))
    XDrawPoint (disp, win, gc, xt, yt);
    xi = xt ;
    yi = yt ;

    th += 1 ;
    th = th - (int) (th / DIVIDE) * DIVIDE ;

    //  WAITING (10 msec) でのタイムアウトを再度インストール
    XtAppAddTimeOut(app_con, WAITING, (XtTimerCallbackProc)drawNextPt, NULL);

    // return False;

}


//  first initalize graphics  (drawin' oval on centre )
void
drawOval(Widget w, caddr_t *client_data, caddr_t *call_data)
{
    double  t;

    //XSetForeground(disp, gc, BlackPixel(disp, 0));

    //. draw centre oval
    for (int i = 0; i < DIVIDE; i++) {
      t = 2*M_PI*i/DIVIDE ;
      xp = WIDTH /2+a*cos(t) ;
      yp = HEIGHT/2-b*sin(t) ;
      XDrawPoint (disp, win, gc, xp, yp);
    }

    //  WAITING (10 msec) でのタイムアウトを再度インストール
    XtAppAddTimeOut(app_con, WAITING, (XtTimerCallbackProc)drawNextPt, NULL);
}

//  quit callback
void
quit_proc()
{
    printf("this client exits normaly.\n");
    printf("thank you !\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    //  1st arg ; for d
    if (argc > 1) {
        if (strcmp(argv[1], "@") != 0) {
            d  = atoi(argv[1]) ;
        }
    }
    //  2nd arg ; for dd
    if (argc > 2) {
        if (strcmp(argv[2], "@") != 0) {
            dd = atoi(argv[2]) ;
        }
    }
    //  3rd arg ; for a
    if (argc > 3) {
        if (strcmp(argv[3], "@") != 0) {
            a  = atoi(argv[3]) ;
        }
    }
    //  4th arg ; for b
    if (argc > 4) {
        if (strcmp(argv[4], "@") != 0) {
            b  = atoi(argv[4]) ;
        }
    }
    
    printf("a = %d , b = %d, d = %d, dd = %d\n", a, b, d, dd) ;
    if (a < b) {
        printf("Sorry, this code cannot handle 'a<b' case ...\n");
        exit(1);
    }

    //  initialize variables 
    e  = sqrt( 1.0 - (b/a)*(b/a) ) ;
    y_0 = a* sqrt( 1 - e*e * sin(th)*sin(th) ) ;  //

    toplevel = XtVaAppInitialize(&app_con, "Spiro", NULL, 0,
				 &argc, argv, fallback_resources, NULL);

    //  toplevel に ウィンドウサイズを設定
    XtSetArg(arg_list[0], XtNwidth,  WIDTH);
    XtSetArg(arg_list[1], XtNheight, HEIGHT);
    XtSetValues(toplevel, arg_list, 2);

    //  mouse button callback for quit this client 
    XtAppAddActions(app_con, actions, XtNumber(actions));

    XtRealizeWidget(toplevel);

    disp = XtDisplay(toplevel);
    win = XtWindow(toplevel);
    gc = XCreateGC(disp, win, 0, 0);
    XSetForeground(disp, gc, BlackPixel(disp, 0));

    //  WAITING の 10 倍でのタイムアウトを設定
    XtAppAddTimeOut(app_con, WAITING*10, (XtTimerCallbackProc)drawOval, NULL);

    XtAppMainLoop(app_con);
}


