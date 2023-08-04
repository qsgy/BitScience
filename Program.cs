using MathNet.Symbolics;

namespace BitScience
{
    internal class Program
    {
        static void Main(string[] args)
        {
 
        }
        public static void SimplifyExpression(string expression)
        {
            SymbolicExpression e = Infix.ParseOrUndefined(expression);
            if (e == null)
            {
                Console.WriteLine("无法识别的表达式：" + expression);
                return;
            }
            var simplifiedExpr = e.ExponentialSimplify();
            Console.WriteLine("简化后的表达式为： " + simplifiedExpr);
        }
        private static void test1()
        {
            Console.WriteLine("Hello, World!");
            Console.WriteLine(((byte)(0xff & 0xff)).ToBinStr());
            Console.WriteLine(((byte)(0xff & 0x00)).ToBinStr());
            Console.WriteLine(((byte)(0xff & 0x01)).ToBinStr());
        }
    }
    public static class helper
    {
        public static string ToBinStr(this byte data)
        {
            return Convert.ToString(data,2).PadLeft(8,'0');
        }
    }
}