
..0.A:�_$Header: /users/bart/hsrc/network/meiko/TRAN/RCS/rte_ra.c,v 1.2 1992/01/14 14:27:02 bart Exp $ �
.DevOpen:�rs��(.DeviceOperate-2)�!�s�(.DeviceClose-2)�!�s�s"��
.DeviceOperate:�`��(..21.A-2)�!��u�v0�pD��..11.A�p`��..6.A�p`��..5.A�..4.A
..11.A:��pC��..15.A�p`��..7.A�..4.A
..15.A:��p`��..10.A�p`��..9.A�p`��..8.A�..4.A
..10.A:�vqt�.driver_Initialise�..3.A
..9.A:�vqt�.driver_Reset�..3.A
..8.A:�vqt�.driver_Analyse�..3.A
..7.A:�vqt�.driver_TestReset�..3.A
..6.A:�vqt�.driver_Boot�..3.A
..5.A:�vqt�.driver_ConditionalReset�..3.A
..4.A:�r0v�
..3.A:��"��
..21.A:�  ��
.DeviceClose:�`�t�q9�p�..23.A�ps�.Close
..23.A:�@�"��
.driver_report:�Z`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�z0�u�v�w�x�y}�..26.A��"��
.driver_fatal:�Z`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�z1�u�v�w�x�y}�..26.A��"��
.driver_Initialise:�`��(..37.A-2)�!��z5�A�w�wRzy�.driver_report�z6�t&�y�.strlen�..29.A�t&��wYzy�.driver_report
..29.A:�z�Є(.initialise_aux1-2)�!�uy�.RmSearchNetwork�..31.A�w!Rzy�.driver_fatal
..31.A:�w!^@y�.Locate��r��..33.A�w"Szy�.driver_fatal
..33.A:�A�@ry�.Open��s��..35.A�ry�.Result2��q�w#Qzy�.driver_fatal
..35.A:�sz�ry�.Close�@{�"��
..37.A:�1.00 ��rte_ra.d driver version %s ��$rte_ra.d driver, ignoring option %s ��0rte_ra.d driver, unable to work in this network ��/NetworkController ��5rte_ra.d driver, failed to locate /NetworkController ��=rte_ra.d driver, failed to open /NetworkController, fault %x ��
.initialise_aux1:�`��(..54.A-2)�%!�����q0Q�pq�p`?��q0Q�pq�p`?�o@�zy�.RmIsNetwork�..39.A�s�tЄ(.initialise_aux1-2)�!�zy�.RmSearchNetwork��"�
..39.A:�zy�.RmGetProcessorPurpose�N$�Ċ..41.A�@�"�
..41.A:�zy�.RmGetProcessorPrivate��r!4���..43.A�zy�.RmGetProcessorId��q�wsy�.driver_report�A�"�
..43.A:�r!50s6���..45.A�@Հ..47.A
..45.A:�r!54s6���..48.A�AՀ..47.A
..48.A:�zy�.RmGetProcessorId��q�w!Qsy�.driver_report�A�"�
..47.A:�r!5u!@����q0!@$�q�zs7���..51.A�@r!5�..53.A
..51.A:�	t0t��r!5�
..53.A:�r!52r!5�@�"��
..54.A:�Brte_ra.d driver, processor %s has wrong number of network drivers ��1rte_ra.d driver has no control over processor %s ��
.driver_Reset:�a��(..82.A-2)�!�!�!{�.NewPort���!|9�!}1J��..56.A�!}1D�!{�.Malloc��}��..56.A�!y0!}��..60.A
..56.A:�@��
..61.A:�!}1s��..62.A�
s!}R�0�r!{�.RmGetProcessorPrivate��q!52s}��s�Ӏ..61.A�
..62.A:�@��
..64.A:�B{��..60.A�1}�@�!y1�!}1D�///O$���B$�@�#�$@�#�|;�~�!y2�!{�.PutMsg�	�q!}�q@���..67.A�|!{�.ReOpen�..69.A�
..67.A:�	v�!y1�!{�.GetMsg�!}�!}0��..70.A�@��
..72.A:�!}1s��..60.A�
s!}R�0�q!{�.RmGetProcessorState��p"@$��pq!{�.RmSetProcessorState�s�Ӏ..72.A�
..70.A:�{��..69.A�|!{�.ReOpen�
..69.A:�{�ۀ..64.A
..60.A:�}�..77.A�}�..77.A�}!{�.Free
..77.A:�~�..80.A�~!{�.FreePort
..80.A:�!�"��
..82.A:��р��   `�
.driver_Analyse:�`��(..84.A-2)�!��psr�.driver_fatal��"��
..84.A:�2rte_ra.d, driver Analyse routine called illegally ��
.driver_TestReset:�`��(..86.A-2)�!��psr�.driver_fatal��"��
..86.A:�4rte_ra.d, driver TestReset routine called illegally ��
.driver_Boot:�`��(..88.A-2)�!��psr�.driver_fatal��"��
..88.A:�4rte_ra.d, driver bootstrap routine called illegally ��
.driver_ConditionalReset:�`��(..90.A-2)�!��psr�.driver_fatal��"��
..90.A:�<rte_ra.d, driver conditional reset routine called illegally ��
..26.A:�x��..dataseg.A 1���`�s0�modnum��q�..dataseg.A��t�..92.A�..93.A
..92.A:�(..0.A-2)�!�p�
..93.A:��"��