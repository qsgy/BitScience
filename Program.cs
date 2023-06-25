namespace BitScience
{
    internal class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello, World!");
            Console.WriteLine(((byte)(0xff&0xff)).ToBinStr());
            Console.WriteLine(((byte)(0xff&0x00)).ToBinStr());
            Console.WriteLine(((byte)(0xff&0x01)).ToBinStr());
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