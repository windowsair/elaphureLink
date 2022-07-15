using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace DriverInstallHelper
{
    class Program
    {
        private static bool isElaphureExist(ref List<string> lineList, int start, int end)
        {
            for (int i = start; i < end; i++)
            {
                if (lineList[i].Contains("elaphureLink"))
                {
                    return true;
                }
            }

            return false;
        }

        private static (int start, int end) getARMSettingRange(ref List<string> lineList)
        {
            int start = -1;
            int end = lineList.Count;

            bool isARMSettingFind = false;

            // get "[ARMADS]"
            for (int i = 0; i < end; i++)
            {
                if (!lineList[i].StartsWith("["))
                {
                    continue;
                }

                // line start with "["
                if (lineList[i].Contains("[ARMADS]"))
                {
                    start = i;
                    isARMSettingFind = true;
                }
                else if (isARMSettingFind)
                {
                    // another setting line start
                    end = i;
                }
            }

            return (start, end);
        }

        private static void getDriverNumUsed(
            ref List<string> lineList,
            int start,
            int end,
            ref SortedDictionary<string, int> mp
        )
        {
            for (int i = start; i < end; i++)
            {
                // TDRVxxxxx=
                if (lineList[i].StartsWith("TDRV"))
                {
                    string TDRVString = lineList[i].Split('=')[0];
                    TDRVString = TDRVString.Replace("TDRV", ""); // get num string
                    mp[TDRVString] = 1;
                }
            }
        }

        private static int modifySettingFile(string settingFileDir)
        {
            // register encoding provider
            //System.Text.Encoding.RegisterProvider(System.Text.CodePagesEncodingProvider.Instance);

            string filePath = String.Format("{0}/TOOLS.INI", settingFileDir);
            List<string> lineList = new List<string>();

            // use windows-1252 codepage
            try
            {
                using (
                    StreamReader sr = new StreamReader(
                        filePath,
                        System.Text.Encoding.GetEncoding(1252),
                        true
                    )
                )
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        lineList.Add(line);
                    }
                }
            }
            catch
            {
                Console.WriteLine("Could not open Keil setting file.");
                return -1;
            }

            var settingRange = getARMSettingRange(ref lineList);
            int start = settingRange.Item1;
            int end = settingRange.Item2;

            if (start == -1)
            {
                Console.WriteLine("Invalid Keil setting file.");
                return -1;
            }

            if (isElaphureExist(ref lineList, start, end))
            {
                Console.WriteLine("Driver setting already exist.");
                return 0;
            }

            SortedDictionary<string, int> driverNumUsed = new SortedDictionary<string, int>();
            getDriverNumUsed(ref lineList, start, end, ref driverNumUsed);

            int driverNum;
            Random r = new Random();
            do
            {
                driverNum = r.Next(100, 1000);
            } while (driverNumUsed.ContainsKey(driverNum.ToString()));

            string driverNumStr = String.Format("TRDV{0}", driverNum);

            // step1: insert driver substring
            for (int i = start; i < end; i++)
            {
                // Drivers for Cortex-M devices
                if (lineList[i].Contains("SARMCM3.DLL"))
                {
                    // SARMCM3.DLL(TDRVxxx, TDRVxxx, TDRVxxx)
                    int insertIndex = lineList[i].IndexOf(")");
                    string addStr = String.Format(",{0}", driverNumStr);
                    string newStr = lineList[i].Insert(insertIndex, addStr);
                    lineList[i] = newStr;
                    break;
                }
            }

            // step2: add driver detail
            string driverDetailStr = String.Format(
                "{0}=BIN\\elaphureLink.dll(\"elaphureLink Debugger\")",
                driverNumStr
            );
            lineList.Insert(end, driverDetailStr);

            // write back
            try
            {
                using (
                    StreamWriter sw = new StreamWriter(
                        filePath,
                        false,
                        System.Text.Encoding.GetEncoding(1252)
                    )
                )
                {
                    foreach (string line in lineList)
                    {
                        sw.WriteLine(line);
                    }
                }
            }
            catch
            {
                Console.WriteLine("Could not write driver setting file.");
            }

            return 0;
        }

        private static int copyDLLFile(string settingFileDir)
        {
            string srcPath,
                dstPath;
            string currentPath = System.IO.Path.GetDirectoryName(
                System.Reflection.Assembly.GetEntryAssembly().Location
            );

            if (
                !File.Exists(Path.Combine(currentPath, "elaphureLink.dll"))
                || !File.Exists(Path.Combine(currentPath, "elaphureRddi.dll"))
            )
            {
                Console.WriteLine("Driver file not exist.");
                return -1;
            }

            srcPath = currentPath + "/elaphureLink.dll";
            dstPath = settingFileDir + "/ARM/BIN/elaphureLink.dll";

            try
            {
                File.Copy(srcPath, dstPath, true);
            }
            catch (System.IO.DirectoryNotFoundException)
            {
                Console.WriteLine("Could not find Keil ARM install directory.");
                return -1;
            }
            catch
            {
                Console.WriteLine("Could not copy driver file.");
                return -1;
            }

            srcPath = currentPath + "/elaphureRddi.dll";
            dstPath = settingFileDir + "/ARM/BIN/elaphureRddi.dll";

            try
            {
                File.Copy(srcPath, dstPath, true);
            }
            catch (System.IO.DirectoryNotFoundException)
            {
                Console.WriteLine("Could not find Keil ARM install directory.");
                return -1;
            }
            catch
            {
                Console.WriteLine("Could not copy driver file.");
                return -1;
            }

            return 0;
        }

        static int Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Invalid parameter.");
                return -1;
            }

            int ret;
            ret = modifySettingFile(args[0]);
            if (ret != 0)
            {
                Console.WriteLine("\nPress any key to exit...");
                Console.ReadKey();
                return ret;
            }

            ret = copyDLLFile(args[0]);
            if (ret != 0)
            {
                Console.WriteLine("\nPress any key to exit...");
                Console.ReadKey();
                return ret;
            }

            return ret;
        }
    }
}
