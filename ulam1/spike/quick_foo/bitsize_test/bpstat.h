#ifndef BPSTAT_H
#define BPSTAT_H

//! Class bpstat
/*! This class implements the running variance algorithm
    by B.P.Welford, presented in Knuth's "Art of Computer Programming"
    Vol 2, page 232, 3rd edition, and www.johndcook.com/standard_deviation.html.
    \n;
    @author elena s. ackley
    @updated: Mon Dec 22 08:58:39 2014
*/
class bpstat
  {
  private:
	  int k;
	  double M,V;
	  double Min, Max;
  public:
    bpstat();
    ~bpstat();

    void clear();
    void update(double x);  //int is sufficient (was double)
    int getNumValues();
    double getMean();
    double getSD();
    double getMin();
    double getMax();
    double calcVar();
  };
#endif //bpstat
