# elaphureLink License

For giving maximum respect to the upstream projects and the licenses they used, the source code of elaphureLink (except for third-party parts) is distributed under the BSD 2-Clause License.

Since this project adopts the CMSIS-DAP related standards, any user who uses this project will also need to agree to the CMSIS-DAP EULA. For the specific scope of this clause, a complete explanation was given in CMSIS-DAP EULA.

The source code from the third-party libraries is distributed under the original license used in the third-party libraries.

Arm, Keil and µVision are registered trademarks of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

This permission notice shall be included in all copies or substantial portions of the Software.


# BSD 2-Clause License

```
Copyright (c) 2022, windowsair
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```


# CMSIS-DAP EULA

```
END USER LICENCE AGREEMENT FOR THE CORTEX MICROCONTROLLER SOFTWARE INTERFACE STANDARD DAP (CMSIS-DAP) SPECIFICATION, CMSIS-DAP FIRMWARE AND RDDI DLL.

THIS END USER LICENCE AGREEMENT ("LICENCE") IS A LEGAL AGREEMENT BETWEEN YOU (EITHER A SINGLE INDIVIDUAL, OR SINGLE LEGAL ENTITY) AND ARM LIMITED ("ARM") FOR THE USE OF THE CMSIS-DAP SPECIFICATION, CMSIS-DAP FIRMWARE, AND A RDDI DLL AS SUCH TERMS ARE DEFINED BELOW (COLLECTIVELY, THE "ARM DELIVERABLES"). ARM IS ONLY WILLING TO LICENSE THE ARM DELIVERABLES TO YOU ON CONDITION THAT YOU ACCEPT ALL OF THE TERMS IN THIS LICENCE. BY CLICKING "I AGREE", OR BY INSTALLING OR OTHERWISE USING OR COPYING THE ARM DELIVERABLES YOU INDICATE THAT YOU AGREE TO BE BOUND BY ALL THE TERMS OF THIS LICENCE. IF YOU DO NOT AGREE TO THE TERMS OF THIS LICENCE, ARM IS UNWILLING TO LICENSE YOU TO USE THE ARM DELIVERABLES AND YOU MAY NOT INSTALL, USE OR COPY THE ARM DELIVERABLES.

"CMSIS-DAP Specification" means any documentation defining the application programming interface, naming and coding conventions of the Cortex Microcontroller Software Interface Standard Debug Access Port ("CMSIS- DAP"). Notwithstanding the foregoing, "CMSIS-DAP Specification" shall not include: (i) the implementation of other published specifications referenced in the CMSIS-DAP Specification; and (ii) any enabling technologies that may be necessary to make or use any product or portions thereof that complies with the CMSIS-DAP Specification, but are not themselves expressly set forth in the CMSIS-DAP Specification (e.g. compiler front ends, code generators, back ends, libraries or other compiler, assembler or linker technologies; validation or debug software or hardware; applications, operating system or driver software; RISC architecture; and processor microarchitecture).

"CMSIS-DAP Firmware" means the C programming language source code accompanying this Licence which implements the functionality of the application programming interface as defined in the CMSIS-DAP Specification and any updates, patches and modifications ARM may make available under the terms of this Licence.

"Firmware" means firmware that complies with the CMSIS-DAP Specification.

"RDDI DLL" means the reference implementation of a device driver accompanying this Licence in object code form and any updates, patches and modifications ARM may agree to make available under the terms of this Licence and is used with targets containing microprocessors manufactured or simulated under licence from ARM.

"Separate Files" means the separate files identified the Schedule.

"Target Connection Product" means a target connection product that complies with the CMSIS-DAP Specification and is used on or with a target containing microprocessors manufactured or simulated under licence from ARM.

1.	LICENCE GRANTS.

(i)	CMSIS-DAP SPECIFICATION
ARM hereby grants to you, subject to the terms and conditions of this Licence, a non-exclusive, non-transferable licence, to use and copy the CMSIS-DAP Specification for the purposes of:

(a)	developing, having developed, manufacturing, having manufactured, offering to sell, selling, supplying, distributing or having distributed a Target Connection Product;
(b)	developing, having developed, subject to clause 1(iv) offering to sell, selling, supplying, distributing or having distributed (directly or through your customers and authorised distributors) Firmware to run on a Target Connection Product; and
(c)	subject to clause 1(iv), distributing and having distributed (directly or through your customers and authorised distributors) the CMSIS-DAP Specification unmodified, with either or both the Target Connection Products and Firmware, developed under the licences granted in this Clause 1(i).

(ii)	CMSIS-DAP FIRMWARE
ARM hereby grants to you, subject to the terms and conditions of this Licence, a non-exclusive, non-transferable licence, to:
(a)	use, copy, and modify the CMSIS-DAP Firmware for the purposes of developing and having developed firmware to run on a Target Connection Product; and

(b)	subject to clause 1 (iv), offer to sell, selling, supply, supplying, distributing or having distributed (directly or through your customers and authorised distributors) CMSIS-DAP Firmware or any modified version created under Clause 1 (ii) (a) in object code form only to run on a Target Connection Product.

(iii)	RDDI DLL
ARM hereby grants to you, subject to the terms and conditions of this Licence, a non-exclusive, non-transferable licence, to:
(a)	use and copy the RDDI DLL for the purpose of connecting a Target Connection Product running CMSIS- DAP compatible firmware (either the Firmware or the firmware created pursuant to Clause 1 (ii)) to software debug tools installed on a host computer running a Windows platform; and
(b)	subject to clause 1 (iv), offer to sell, selling, supplying, distributing or having distributed (directly or through your customers and authorised distributors) the RDDI DLL in object code form only.

(iv)	CONDITIONS ON DISTRIBUTION AND REDISTRIBUTION
If you are authorised and choose to distribute (directly or through your customers and authorised distributors) the CMSIS-DAP Specification , Firmware, CMSIS-DAP Firmware or any modified version thereof, or the RDDI DLL, you agree; (a) to ensure that they are licensed for use with targets containing microprocessors manufactured or simulated under licence from ARM; (b) to preserve any copyright notices which are included with the CMSIS-DAP Specification, CMSIS-DAP Firmware, and include valid copyright notices in; (i) any modified version of the CMSIS-DAP Firmware; and (ii) the Firmware; (c) not to use ARM’s name, logo or trademarks to market -any or all of the CMSIS-DAP Specification, Firmware, CMSIS-DAP Firmware or any modified version therof, the RDDI DLL or the Target Connection Products; (d) to ensure your customers and authorised distributors comply with this Clause 1 (iv).

2.	RESTRICTIONS ON USE OF THE ARM DELIVERABLES.

PERMITTED USERS: The ARM Deliverables shall be used only by you (either a single individual, or single legal entity) your employees, or by your on-site bona fide sub-contractors for whose acts and omissions you hereby agree to be responsible to ARM for to the same extent as you are for your employees, and provided always that such sub-contractors; (i) are contractually obligated to use the ARM Deliverables only for your benefit, and (ii) agree to assign all their work product and any rights they create therein in the supply of such work to you.

COPYRIGHT AND RESERVATION OF RIGHTS: The ARM Deliverables are owned by ARM or its licensors and are protected by copyright and other intellectual property laws and international treaties. The ARM Deliverables are licensed not sold. Except as expressly licensed herein, you acquire no right, title or interest in the ARM Deliverables or any intellectual property therein. In no event shall the licences granted herein be construed as granting you, expressly or by implication, estoppels or otherwise, a licence to use any ARM technology except the ARM Deliverables.


3.	SUPPORT.

ARM is not obligated to support the ARM Deliverables but may do so entirely at ARM's discretion.

4.	NO WARRANTY

YOU AGREE THAT THE ARM DELIVERABLES ARE LICENSED "AS IS", AND THAT ARM EXPRESSLY DISCLAIMS ALL REPRESENTATIONS, WARRANTIES, CONDITIONS OR OTHER TERMS, EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON- INFRINGEMENT, SATISFACTORY QUALITY, AND FITNESS FOR A PARTICULAR PURPOSE. THE ARM DELIVERABLES MAY CONTAIN ERRORS.

5.	LIMITATION OF LIABILITY.

THE MAXIMUM LIABILITY OF ARM TO YOU IN AGGREGATE FOR ALL CLAIMS MADE AGAINST ARM IN CONTRACT, TORT OR OTHERWISE UNDER OR IN CONNECTION WITH THE SUBJECT MATTER OF THIS LICENCE SHALL NOT EXCEED THE GREATER OF (I) THE TOTAL OF SUMS PAID BY YOU TO ARM (IF

ANY) FOR THIS LICENCE AND (II) US$10.00. THE LIMITATIONS, EXCLUSIONS AND DISCLAIMERS IN THIS LICENCE SHALL APPLY TO THE MAXIMUM EXTENT ALLOWED BY APPLICABLE LAW.

6.	THIRD PARTY RIGHTS.

The Separate Files are delivered subject to and your use is governed by their own separate licence agreements. This Licence does not apply to such Separate Files and they are not included in the term "ARM Deliverables" under this Licence. You agree to comply with all terms and conditions imposed on you in respect of such Separate Files including those identified in the Schedule ("Third Party Terms").

ARM HEREBY DISCLAIMS ANY AND ALL WARRANTIES EXPRESS OR IMPLIED FROM ANY THIRD PARTIES REGARDING ANY SEPARATE FILES, ANY THIRD PARTY MATERIALS INCLUDED IN THE ARM DELIVERABLES, ANY THIRD PARTY MATERIALS FROM WHICH THE ARM DELIVERABLES ARE DERIVED (COLLECTIVELY "OTHER CODE"), AND THE USE OF ANY OR ALL THE OTHER CODE IN CONNECTION WITH THE ARM DELIEVRABLES, INCLUDING (WITHOUT LIMITATION) ANY WARRANTIES OF SATISFACTORY QUALITY OR FITNESS FOR A PARTICULAR PURPOSE.

NO THIRD PARTY LICENSORS OF OTHER CODE SHALL HAVE ANY LIABILITY FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED AND WHETHER MADE UNDER CONTRACT, TORT OR OTHER LEGAL THEORY, ARISING IN ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE OTHER CODE OR THE EXERCISE OF ANY RIGHTS GRANTED UNDER EITHER OR BOTH THIS LICENCE AND THE LEGAL TERMS APPLICABLE TO ANY SEPARATE FILES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

The CMSIS-DAP Firmware includes material supplied by third party device manufacturers for use with their relevant target devices. To the extent such materials are identified as such in the CMSIS-DAP Firmware (for example in a "Help About" box or on-screen copyright message or by including the name of the relevant manufacturer or device), you agree to use such materials (if at all) only to develop a Target Connection Product or Firmware for the relevant devices of that vendor.

7.	US Government Restrictions: Use, duplication, reproduction, release, modification, disclosure or transfer of this commercial product and accompanying documentation is restricted in accordance with the terms of this Licence.

8.	TERM AND TERMINATION.

8.1	This Licence shall remain in force until terminated in accordance with the terms of Clause 8.2 or Clause 8.3 below.

8.2	Without prejudice to any of its other rights if you are in breach of any of the terms and conditions of this Licence then ARM may terminate this Licence immediately upon giving written notice to you. You may terminate this Licence at any time.

8.3	This Licence shall immediately terminate and shall be unavailable to you if you or any party affiliated to you asserts any patents against ARM, ARM affiliates, third parties who have a valid licence from ARM for the ARM Deliverables, or any customers or distributors of any of them based upon a claim that your (or your affiliate) patent is Necessary to implement the CMSIS-DAP Specification. In this Licence; (i) "affiliate" means any entity controlling, controlled by or under common control with a party (in fact or in law, via voting securities, management control or otherwise) and "affiliated" shall be construed accordingly; (ii) "assert" means to allege infringement in legal or administrative proceedings, or proceedings before any other competent trade, arbitral or international authority; (iii) "Necessary" means with respect to any claims of any patent, those claims which, without the appropriate permission of the patent owner, will be infringed when implementing the CMSIS-DAP Specification because no alternative, commercially reasonable, non-infringing way of implementing the CMSIS- DAP Specification.

8.4	Upon termination of this Licence, you shall stop using the ARM Deliverables and destroy all copies of the ARM Deliverables in your possession. The provisions of clauses 5, 6, 7, and 8 shall survive termination of this Licence.

9.	GENERAL.

This Licence is governed by English Law. Except where ARM agrees otherwise in a written contract signed by you and ARM, this is the only agreement between you and ARM relating to the ARM Deliverables and it may only be modified by written agreement between you and ARM. Except as expressly agreed in writing, this Licence may not be modified by purchase orders, advertising or other representation by any person. If any clause or sentence in this Licence is held by a court of law to be illegal or unenforceable the remaining provisions of this Licence shall not be affected thereby. The failure by ARM to enforce any of the provisions of this Licence, unless waived in writing, shall not constitute a waiver of ARM's rights to enforce such provision or any other provision of this Licence in the future. This Licence may not be assigned without the prior written consent of ARM.

```

# Third-party libraries

- Asio, https://github.com/chriskohlhoff/asio
- AutoUpdaterdotNET, https://github.com/ravibpatel/AutoUpdater.NET
- GitInfo, https://github.com/devlooped/GitInfo
- ModernWpf, https://github.com/Kinnara/ModernWpf
- NLog, https://github.com/NLog/NLog
- WindowsCommunityToolkit, https://github.com/CommunityToolkit/WindowsCommunityToolkit
