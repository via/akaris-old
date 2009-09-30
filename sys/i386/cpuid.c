/*Small cpuid feature checking library*/


int cpuHasFeature(int feature) {

  int code = 1;
  int a, d;
  __asm__ volatile("cpuid":"=a"(a),"=d"(d):"0"(code):"ecx","ebx");
  return d & feature;
}
