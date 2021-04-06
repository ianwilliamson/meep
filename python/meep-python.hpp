namespace meep {

// like custom_src_time, but using Python function object, with proper reference counting
class custom_py_src_time : public src_time {
public:
  custom_py_src_time(PyObject *fun, double st = -infinity, double et = infinity,
                     std::complex<double> f = 0)
      : func(fun), freq(f), start_time(float(st)), end_time(float(et)) {
    Py_INCREF(func);
  }
  virtual ~custom_py_src_time() { Py_DECREF(func); }

  virtual std::complex<double> current(double time, double dt) const {
    if (is_integrated)
      return src_time::current(time, dt);
    else
      return dipole(time);
  }
  virtual std::complex<double> dipole(double time) const {
    float rtime = float(time);
    if (rtime >= start_time && rtime <= end_time) {
      PyObject *py_t = PyFloat_FromDouble(time);
      PyObject *pyres = PyObject_CallFunctionObjArgs(func, py_t, NULL);
      double real = PyComplex_RealAsDouble(pyres);
      double imag = PyComplex_ImagAsDouble(pyres);
      std::complex<double> ret(real, imag);
      Py_DECREF(py_t);
      Py_DECREF(pyres);
      return ret;
    }
    else
      return 0.0;
  }
  virtual double last_time() const { return end_time; };
  virtual src_time *clone() const {
    Py_INCREF(func); // default copy constructor doesn't incref
    return new custom_py_src_time(*this);
  }
  virtual bool is_equal(const src_time &t) const {
    const custom_py_src_time *tp = dynamic_cast<const custom_py_src_time *>(&t);
    if (tp)
      return (tp->start_time == start_time && tp->end_time == end_time && tp->func == func &&
              tp->freq == freq);
    else
      return 0;
  }
  virtual std::complex<double> frequency() const { return freq; }
  virtual void set_frequency(std::complex<double> f) { freq = f; }

private:
  PyObject *func;
  std::complex<double> freq;
  double start_time, end_time;
};

} // namespace meep
