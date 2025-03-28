using System;
using System.Runtime.InteropServices;
using System.Text;

namespace DotNetRouter
{
    internal class Program
    {
        [DllImport("RouterDll.dll", EntryPoint = "try_route_packet", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool TryRoutePacket(string packet, string routerCallsign, string routerPath, StringBuilder routedPacket, out int bufferSize);
        
        public static string RoutePacket(string packet, string routerCallsign, string routerPath)
        {
            int bufferSize = 1024;
            StringBuilder routedPacketBuilder = new StringBuilder(bufferSize);
            if (TryRoutePacket(packet, routerCallsign, routerPath, routedPacketBuilder, out bufferSize))
            {
                return routedPacketBuilder.ToString(0, bufferSize);
            }
            else
            {
                throw new Exception("Routing failed.");
            }
        }

        static void Main(string[] args)
        {
            string routedPacket = RoutePacket("N0CALL>APRS,WIDE1-1,WIDE2-1:data", "DIGI", "WIDE1-1,WIDE2-2");
            Console.WriteLine(routedPacket);
        }
    }
}
