local strokesData = {
  stroke1 = {
    x = { 101.98581452552, 104.31802452552, 106.64826452552, 108.97456452552, 111.29494452552, 113.60745452552, 115.91012452552, 118.20101452552, 120.47819452552, 122.73971452552, 124.98367452552, 127.20819452552, 129.41135452552, 131.59132452552, 133.74623452552, 135.87427452552, 137.97363452552, 140.04256452552, 142.07928452552, 144.08207452552, 146.04925452552, 147.97914452552, 149.87012452552, 151.72059452552, 153.52898452552, 155.29376452552, 157.01343452552, 158.68654452552, 160.31167452552, 161.88746452552, 163.41257452552, 164.88570452552, 166.30561452552, 167.67110452552, 168.98102452552, 170.23425452552, 171.42974452552, 172.56646452552, 173.64348452552, 174.65986452552, 175.61475452552, 176.50734452552, 177.33689452552, 178.10267452552, 178.80405452552, 179.44045452552, 180.01129452552, 180.51614452552, 180.95453452552, 181.32611452552, 181.63056452552, 181.86761452552, 182.03709452552, 182.13882452552, 182.17274452552, 182.13882452552, 182.03709452552, 181.86761452552, 181.63056452552, 181.32611452552, 180.95453452552, 180.51614452552, 180.01129452552, 179.44045452552, 178.80405452552, 178.10267452552, 177.33689452552, 176.50734452552, 175.61475452552, 174.65986452552, 173.64348452552, 172.56646452552, 171.42974452552, 170.23425452552, 168.98102452552, 167.67110452552, 166.30561452552, 164.88570452552, 163.41257452552, 161.88746452552, 160.31167452552, 158.68654452552, 157.01343452552, 155.29376452552, 153.52898452552, 151.72059452552, 149.87012452552, 147.97914452552, 146.04925452552, 144.08207452552, 142.07928452552, 140.04256452552, 137.97363452552, 135.87427452552, 133.74623452552, 131.59132452552, 129.41135452552, 127.20819452552, 124.98367452552, 122.73971452552, 120.47819452552, 118.20101452552, 115.91012452552, 113.60745452552, 111.29494452552, 108.97456452552, 106.64826452552, 104.31802452552, 101.98581452552, 99.653594525524, 97.323354525524, 94.997054525524, 92.676674525524, 90.364164525524, 88.061484525524, 85.770604525524, 83.493424525524, 81.231904525524, 78.987934525524, 76.763424525524, 74.560264525524, 72.380294525524, 70.225384525524, 68.097344525524, 65.997974525524, 63.929054525524, 61.892344525524, 59.889534525524, 57.922354525524, 55.992464525524, 54.101484525524, 52.251014525524, 50.442634525524, 48.677854525524, 46.958194525524, 45.285074525524, 43.659944525524, 42.084144525524, 40.559054525524, 39.085914525524, 37.666004525524, 36.300514525524, 34.990594525524, 33.737364525524, 32.541874525524, 31.405154525524, 30.328144525524, 29.311754525524, 28.356864525524, 27.464274525524, 26.634724525524, 25.868934525524, 25.167554525524, 24.531174525524, 23.960314525524, 23.455474525524, 23.017084525524, 22.645514525524, 22.341064525524, 22.104004525524, 21.934534525524, 21.832784525524, 21.798864525524, 21.832784525524, 21.934534525524, 22.104004525524, 22.341064525524, 22.645514525524, 23.017084525524, 23.455474525524, 23.960314525524, 24.531174525524, 25.167554525524, 25.868934525524, 26.634724525524, 27.464274525524, 28.356864525524, 29.311754525524, 30.328144525524, 31.405154525524, 32.541874525524, 33.737364525524, 34.990594525524, 36.300514525524, 37.666004525524, 39.085914525524, 40.559054525524, 42.084144525524, 43.659944525524, 45.285074525524, 46.958194525524, 48.677854525524, 50.442634525524, 52.251014525524, 54.101484525524, 55.992464525524, 57.922354525524, 59.889534525524, 61.892344525524, 63.929054525524, 65.997974525524, 68.097344525524, 70.225384525524, 72.380294525524, 74.560264525524, 76.763424525524, 78.987934525524, 81.231904525524, 83.493424525524, 85.770604525524, 88.061484525524, 90.364164525524, 92.676674525524, 94.997054525524, 97.323354525524, 99.653594525524, 101.98581452552 },
    y = { 75.214868720324, 75.225818720324, 75.258628720324, 75.313298720324, 75.389758720324, 75.487958720324, 75.607818720324, 75.749228720324, 75.912068720324, 76.096208720324, 76.301478720324, 76.527718720324, 76.774738720324, 77.042308720324, 77.330228720324, 77.638238720324, 77.966088720324, 78.313498720324, 78.680158720324, 79.065778720324, 79.470018720324, 79.892548720324, 80.333008720324, 80.791018720324, 81.266188720324, 81.758128720324, 82.266418720324, 82.790628720324, 83.330308720324, 83.885008720324, 84.454258720324, 85.037578720324, 85.634468720324, 86.244428720324, 86.866938720324, 87.501468720324, 88.147498720324, 88.804468720324, 89.471818720324, 90.148998720324, 90.835428720324, 91.530508720324, 92.233688720324, 92.944348720324, 93.661888720324, 94.385708720324, 95.115188720324, 95.849718720324, 96.588678720324, 97.331428720324, 98.077358720324, 98.825818720324, 99.576198720324, 100.32784872032, 101.08012872032, 101.83240872032, 102.58405872032, 103.33442872032, 104.08289872032, 104.82882872032, 105.57157872032, 106.31053872032, 107.04506872032, 107.77454872032, 108.49836872032, 109.21590872032, 109.92656872032, 110.62973872032, 111.32483872032, 112.01125872032, 112.68843872032, 113.35578872032, 114.01275872032, 114.65877872032, 115.29331872032, 115.91583872032, 116.52578872032, 117.12267872032, 117.70599872032, 118.27524872032, 118.82994872032, 119.36962872032, 119.89383872032, 120.40212872032, 120.89406872032, 121.36924872032, 121.82724872032, 122.26770872032, 122.69023872032, 123.09448872032, 123.48009872032, 123.84676872032, 124.19416872032, 124.52201872032, 124.83002872032, 125.11793872032, 125.38551872032, 125.63252872032, 125.85876872032, 126.06404872032, 126.24818872032, 126.41102872032, 126.55243872032, 126.67229872032, 126.77049872032, 126.84696872032, 126.90162872032, 126.93444872032, 126.94538872032, 126.93444872032, 126.90162872032, 126.84696872032, 126.77049872032, 126.67229872032, 126.55243872032, 126.41102872032, 126.24818872032, 126.06404872032, 125.85876872032, 125.63252872032, 125.38551872032, 125.11793872032, 124.83002872032, 124.52201872032, 124.19416872032, 123.84676872032, 123.48009872032, 123.09448872032, 122.69023872032, 122.26770872032, 121.82724872032, 121.36924872032, 120.89406872032, 120.40212872032, 119.89383872032, 119.36962872032, 118.82994872032, 118.27524872032, 117.70599872032, 117.12267872032, 116.52578872032, 115.91583872032, 115.29331872032, 114.65877872032, 114.01275872032, 113.35578872032, 112.68843872032, 112.01125872032, 111.32483872032, 110.62973872032, 109.92656872032, 109.21590872032, 108.49836872032, 107.77454872032, 107.04506872032, 106.31053872032, 105.57157872032, 104.82882872032, 104.08289872032, 103.33442872032, 102.58405872032, 101.83240872032, 101.08012872032, 100.32784872032, 99.576198720324, 98.825818720324, 98.077358720324, 97.331428720324, 96.588678720324, 95.849718720324, 95.115188720324, 94.385708720324, 93.661888720324, 92.944348720324, 92.233688720324, 91.530508720324, 90.835428720324, 90.148998720324, 89.471818720324, 88.804468720324, 88.147498720324, 87.501468720324, 86.866938720324, 86.244428720324, 85.634468720324, 85.037578720324, 84.454258720324, 83.885008720324, 83.330308720324, 82.790628720324, 82.266418720324, 81.758128720324, 81.266188720324, 80.791018720324, 80.333008720324, 79.892548720324, 79.470018720324, 79.065778720324, 78.680158720324, 78.313498720324, 77.966088720324, 77.638238720324, 77.330228720324, 77.042308720324, 76.774738720324, 76.527718720324, 76.301478720324, 76.096208720324, 75.912068720324, 75.749228720324, 75.607818720324, 75.487958720324, 75.389758720324, 75.313298720324, 75.258628720324, 75.225818720324, 75.214868720324 },
    pressure = {},
    tool = "pen",
    color = 10820909,
    width = 1.41,
    fill = 0,
    lineStyle = "dot",
  },
  stroke2 = {
    x = { 82.305764525524, 123.59331452552 },
    y = { 123.80016872032, 77.574048720324 },
    pressure = {},
    tool = "pen",
    color = 8421504,
    width = 0.85,
    fill = 0,
    lineStyle = "dashdot",
  },
  stroke3 = {
    x = { 101.98581452552, 104.31802452552, 106.64826452552, 108.97456452552, 111.29494452552, 113.60745452552, 115.91012452552, 118.20101452552, 120.47819452552, 122.73971452552, 124.98367452552, 127.20819452552, 129.41135452552, 131.59132452552, 133.74623452552, 135.87427452552, 137.97363452552, 140.04256452552, 142.07928452552, 144.08207452552, 146.04925452552, 147.97914452552, 149.87012452552, 151.72059452552, 153.52898452552, 155.29376452552, 157.01343452552, 158.68654452552, 160.31167452552, 161.88746452552, 163.41257452552, 164.88570452552, 166.30561452552, 167.67110452552, 168.98102452552, 170.23425452552, 171.42974452552, 172.56646452552, 173.64348452552, 174.65986452552, 175.61475452552, 176.50734452552, 177.33689452552, 178.10267452552, 178.80405452552, 179.44045452552, 180.01129452552, 180.51614452552, 180.95453452552, 181.32611452552, 181.63056452552, 181.86761452552, 182.03709452552, 182.13882452552, 182.17274452552, 182.13882452552, 182.03709452552, 181.86761452552, 181.63056452552, 181.32611452552, 180.95453452552, 180.51614452552, 180.01129452552, 179.44045452552, 178.80405452552, 178.10267452552, 177.33689452552, 176.50734452552, 175.61475452552, 174.65986452552, 173.64348452552, 172.56646452552, 171.42974452552, 170.23425452552, 168.98102452552, 167.67110452552, 166.30561452552, 164.88570452552, 163.41257452552, 161.88746452552, 160.31167452552, 158.68654452552, 157.01343452552, 155.29376452552, 153.52898452552, 151.72059452552, 149.87012452552, 147.97914452552, 146.04925452552, 144.08207452552, 142.07928452552, 140.04256452552, 137.97363452552, 135.87427452552, 133.74623452552, 131.59132452552, 129.41135452552, 127.20819452552, 124.98367452552, 122.73971452552, 120.47819452552, 118.20101452552, 115.91012452552, 113.60745452552, 111.29494452552, 108.97456452552, 106.64826452552, 104.31802452552, 101.98581452552, 99.653594525524, 97.323354525524, 94.997054525524, 92.676674525524, 90.364164525524, 88.061484525524, 85.770604525524, 83.493424525524, 81.231904525524, 78.987934525524, 76.763424525524, 74.560264525524, 72.380294525524, 70.225384525524, 68.097344525524, 65.997974525524, 63.929054525524, 61.892344525524, 59.889534525524, 57.922354525524, 55.992464525524, 54.101484525524, 52.251014525524, 50.442634525524, 48.677854525524, 46.958194525524, 45.285074525524, 43.659944525524, 42.084144525524, 40.559054525524, 39.085914525524, 37.666004525524, 36.300514525524, 34.990594525524, 33.737364525524, 32.541874525524, 31.405154525524, 30.328144525524, 29.311754525524, 28.356864525524, 27.464274525524, 26.634724525524, 25.868934525524, 25.167554525524, 24.531174525524, 23.960314525524, 23.455474525524, 23.017084525524, 22.645514525524, 22.341064525524, 22.104004525524, 21.934534525524, 21.832784525524, 21.798864525524, 21.832784525524, 21.934534525524, 22.104004525524, 22.341064525524, 22.645514525524, 23.017084525524, 23.455474525524, 23.960314525524, 24.531174525524, 25.167554525524, 25.868934525524, 26.634724525524, 27.464274525524, 28.356864525524, 29.311754525524, 30.328144525524, 31.405154525524, 32.541874525524, 33.737364525524, 34.990594525524, 36.300514525524, 37.666004525524, 39.085914525524, 40.559054525524, 42.084144525524, 43.659944525524, 45.285074525524, 46.958194525524, 48.677854525524, 50.442634525524, 52.251014525524, 54.101484525524, 55.992464525524, 57.922354525524, 59.889534525524, 61.892344525524, 63.929054525524, 65.997974525524, 68.097344525524, 70.225384525524, 72.380294525524, 74.560264525524, 76.763424525524, 78.987934525524, 81.231904525524, 83.493424525524, 85.770604525524, 88.061484525524, 90.364164525524, 92.676674525524, 94.997054525524, 97.323354525524, 99.653594525524, 101.98581452552 },
    y = { 75.214868720324, 75.225818720324, 75.258628720324, 75.313298720324, 75.389758720324, 75.487958720324, 75.607818720324, 75.749228720324, 75.912068720324, 76.096208720324, 76.301478720324, 76.527718720324, 76.774738720324, 77.042308720324, 77.330228720324, 77.638238720324, 77.966088720324, 78.313498720324, 78.680158720324, 79.065778720324, 79.470018720324, 79.892548720324, 80.333008720324, 80.791018720324, 81.266188720324, 81.758128720324, 82.266418720324, 82.790628720324, 83.330308720324, 83.885008720324, 84.454258720324, 85.037578720324, 85.634468720324, 86.244428720324, 86.866938720324, 87.501468720324, 88.147498720324, 88.804468720324, 89.471818720324, 90.148998720324, 90.835428720324, 91.530508720324, 92.233688720324, 92.944348720324, 93.661888720324, 94.385708720324, 95.115188720324, 95.849718720324, 96.588678720324, 97.331428720324, 98.077358720324, 98.825818720324, 99.576198720324, 100.32784872032, 101.08012872032, 101.83240872032, 102.58405872032, 103.33442872032, 104.08289872032, 104.82882872032, 105.57157872032, 106.31053872032, 107.04506872032, 107.77454872032, 108.49836872032, 109.21590872032, 109.92656872032, 110.62973872032, 111.32483872032, 112.01125872032, 112.68843872032, 113.35578872032, 114.01275872032, 114.65877872032, 115.29331872032, 115.91583872032, 116.52578872032, 117.12267872032, 117.70599872032, 118.27524872032, 118.82994872032, 119.36962872032, 119.89383872032, 120.40212872032, 120.89406872032, 121.36924872032, 121.82724872032, 122.26770872032, 122.69023872032, 123.09448872032, 123.48009872032, 123.84676872032, 124.19416872032, 124.52201872032, 124.83002872032, 125.11793872032, 125.38551872032, 125.63252872032, 125.85876872032, 126.06404872032, 126.24818872032, 126.41102872032, 126.55243872032, 126.67229872032, 126.77049872032, 126.84696872032, 126.90162872032, 126.93444872032, 126.94538872032, 126.93444872032, 126.90162872032, 126.84696872032, 126.77049872032, 126.67229872032, 126.55243872032, 126.41102872032, 126.24818872032, 126.06404872032, 125.85876872032, 125.63252872032, 125.38551872032, 125.11793872032, 124.83002872032, 124.52201872032, 124.19416872032, 123.84676872032, 123.48009872032, 123.09448872032, 122.69023872032, 122.26770872032, 121.82724872032, 121.36924872032, 120.89406872032, 120.40212872032, 119.89383872032, 119.36962872032, 118.82994872032, 118.27524872032, 117.70599872032, 117.12267872032, 116.52578872032, 115.91583872032, 115.29331872032, 114.65877872032, 114.01275872032, 113.35578872032, 112.68843872032, 112.01125872032, 111.32483872032, 110.62973872032, 109.92656872032, 109.21590872032, 108.49836872032, 107.77454872032, 107.04506872032, 106.31053872032, 105.57157872032, 104.82882872032, 104.08289872032, 103.33442872032, 102.58405872032, 101.83240872032, 101.08012872032, 100.32784872032, 99.576198720324, 98.825818720324, 98.077358720324, 97.331428720324, 96.588678720324, 95.849718720324, 95.115188720324, 94.385708720324, 93.661888720324, 92.944348720324, 92.233688720324, 91.530508720324, 90.835428720324, 90.148998720324, 89.471818720324, 88.804468720324, 88.147498720324, 87.501468720324, 86.866938720324, 86.244428720324, 85.634468720324, 85.037578720324, 84.454258720324, 83.885008720324, 83.330308720324, 82.790628720324, 82.266418720324, 81.758128720324, 81.266188720324, 80.791018720324, 80.333008720324, 79.892548720324, 79.470018720324, 79.065778720324, 78.680158720324, 78.313498720324, 77.966088720324, 77.638238720324, 77.330228720324, 77.042308720324, 76.774738720324, 76.527718720324, 76.301478720324, 76.096208720324, 75.912068720324, 75.749228720324, 75.607818720324, 75.487958720324, 75.389758720324, 75.313298720324, 75.258628720324, 75.225818720324, 75.214868720324 },
    pressure = {},
    tool = "pen",
    color = 10820909,
    width = 1.41,
    fill = 0,
    lineStyle = "dot",
  },
  stroke4 = {
    x = { 80.177734525524, 80.033054525524, 79.870024525524, 79.725864525524, 79.600664525524, 79.494554525524, 79.407614525524, 79.339914525524, 79.291524525524, 79.262464525524, 79.252784525524, 79.262464525524, 79.291524525524, 79.339914525524, 79.407614525524, 79.494554525524, 79.600664525524, 79.725864525524, 79.870024525524, 80.033054525524, 80.214784525524, 80.415074525524, 80.633764525524, 80.870664525524, 81.125564525524, 81.398254525524, 81.688494525524, 81.996064525524, 82.320674525524, 82.662074525524, 83.019964525524, 83.394044525524, 83.783984525524, 84.189464525524, 84.610154525524, 85.045684525524, 85.495684525524, 85.959774525524, 86.437564525524, 86.928654525524, 87.432624525524, 87.949044525524, 88.477494525524, 89.017504525524, 89.568624525524, 90.130394525524, 90.702334525524, 91.283964525524, 91.874784525524, 92.474314525524, 93.082014525524, 93.697404525524, 94.319934525524, 94.949094525524, 95.584354525524, 96.225164525524, 96.870994525524, 97.521294525524, 98.175504525524, 98.833084525524, 99.493464525524, 100.15610452552, 100.82042452552, 101.48587452552, 102.15189452552, 102.81790452552, 103.48335452552, 104.14768452552, 104.81032452552, 105.47070452552, 106.12828452552, 106.78249452552, 107.43279452552, 108.07862452552, 108.71943452552, 109.35469452552, 109.98385452552, 110.60639452552, 111.22176452552, 111.82948452552, 112.42899452552, 113.01982452552, 113.60145452552, 114.17339452552, 114.73516452552, 115.28628452552, 115.82629452552, 116.35473452552, 116.87116452552, 117.37513452552, 117.86621452552, 118.34401452552, 118.80810452552, 119.25811452552, 119.69363452552, 120.11431452552, 120.51980452552, 120.90975452552, 121.28382452552, 121.64171452552, 121.98310452552, 122.30773452552, 122.61529452552, 122.90554452552, 123.17823452552, 123.43313452552, 123.67002452552, 123.84376452552 },
    y = { 124.94149872032, 123.15566872032, 120.89496872032, 118.61862872032, 116.32857872032, 114.02674872032, 111.71508872032, 109.39554872032, 107.07009872032, 104.74071872032, 102.40934872032, 100.07798872032, 97.748598720324, 95.423158720324, 93.103618720324, 90.791958720324, 88.490128720324, 86.200068720324, 83.923728720324, 81.663028720324, 79.419888720324, 77.196188720324, 74.993828720324, 72.814668720324, 70.660538720324, 68.533278720324, 66.434678720324, 64.366508720324, 62.330538720324, 60.328478720324, 58.362018720324, 56.432828720324, 54.542538720324, 52.692748720324, 50.885028720324, 49.120898720324, 47.401848720324, 45.729348720324, 44.104808720324, 42.529598720324, 41.005048720324, 39.532458720324, 38.113058720324, 36.748068720324, 35.438628720324, 34.185858720324, 32.990808720324, 31.854498720324, 30.777878720324, 29.761868720324, 28.807328720324, 27.915058720324, 27.085818720324, 26.320318720324, 25.619188720324, 24.983028720324, 24.412388720324, 23.907738720324, 23.469498720324, 23.098058720324, 22.793728720324, 22.556748720324, 22.387338720324, 22.285638720324, 22.251728720324, 22.285638720324, 22.387338720324, 22.556748720324, 22.793728720324, 23.098058720324, 23.469498720324, 23.907738720324, 24.412388720324, 24.983028720324, 25.619188720324, 26.320318720324, 27.085818720324, 27.915058720324, 28.807328720324, 29.761868720324, 30.777878720324, 31.854498720324, 32.990808720324, 34.185858720324, 35.438628720324, 36.748068720324, 38.113058720324, 39.532458720324, 41.005048720324, 42.529598720324, 44.104808720324, 45.729348720324, 47.401848720324, 49.120898720324, 50.885028720324, 52.692748720324, 54.542538720324, 56.432828720324, 58.362018720324, 60.328478720324, 62.330538720324, 64.366508720324, 66.434678720324, 68.533278720324, 70.660538720324, 72.814668720324, 74.993828720324, 76.743458720324 },
    pressure = {},
    tool = "pen",
    color = 10820909,
    width = 1.41,
    fill = 0,
    lineStyle = "dashdot",
  },
  stroke5 = {
    x = { 102.41383452552, 102.41383452552 },
    y = { 22.485578720324, 102.05871872032 },
    pressure = {},
    tool = "pen",
    color = 8421504,
    width = 0.85,
    fill = 0,
    lineStyle = "dashdot",
  },
  stroke6 = {
    x = { 23.064494525524, 180.91204452552 },
    y = { 101.63737872032, 101.63737872032 },
    pressure = {},
    tool = "pen",
    color = 8421504,
    width = 0.85,
    fill = 0,
    lineStyle = "dot",
  },
  stroke7 = {
    x = { 102.41383452552, 102.41383452552 },
    y = { 118.97473872032, 119.90043872032 },
    pressure = {},
    tool = "pen",
    color = 8421504,
    width = 0.85,
    fill = 0,
    lineStyle = "dot",
  },
  stroke8 = {
    x = { 21.832194525524, 21.864554525524, 21.966264525524, 22.135674525524, 22.372644525524, 22.676984525524, 23.048424525524, 23.486654525524, 23.991304525524, 24.561954525524, 25.198104525524, 25.899234525524, 26.664744525524, 27.493984525524, 28.386254525524, 29.340794525524, 30.356804525524, 31.433424525524, 32.569734525524, 33.764784525524, 35.017554525524, 36.326994525524, 37.691984525524, 39.111374525524, 40.583974525524, 42.108514525524, 43.683734525524, 45.308274525524, 46.980774525524, 48.699814525524, 50.463944525524, 52.271664525524, 54.121454525524, 56.011744525524, 57.940934525524, 59.907394525524, 61.909464525524, 63.945434525524, 66.013594525524, 68.112194525524, 70.239464525524, 72.393584525524, 74.572754525524, 76.775114525524, 78.998804525524, 81.241954525524, 83.502654525524, 85.778994525524, 88.069044525524, 90.370874525524, 92.682544525524, 95.002074525524, 97.327524525524, 99.656914525524, 101.98827452552, 104.31963452552, 106.64902452552, 108.97447452552, 111.29400452552, 113.60566452552, 115.90749452552, 118.19755452552, 120.47389452552, 122.73459452552, 124.97773452552, 127.20143452552, 129.40379452552, 131.58296452552, 133.73708452552, 135.86434452552, 137.96294452552, 140.03111452552, 142.06708452552, 144.06914452552, 146.03560452552, 147.96479452552, 149.85508452552, 151.70487452552, 153.51260452552, 155.27673452552, 156.99577452552, 158.66827452552, 160.29281452552, 161.86802452552, 163.39257452552, 164.86517452552, 166.28456452552, 167.64955452552, 168.95899452552, 170.21176452552, 171.40681452552, 172.54312452552, 173.61974452552, 174.63575452552, 175.59029452552, 176.48256452552, 177.31180452552, 178.07730452552, 178.77843452552, 179.41459452552, 179.98523452552, 180.48989452552, 180.92812452552, 181.29956452552, 181.60390452552, 181.84087452552, 182.01028452552, 182.11198452552, 182.07173452552, 182.02007452552, 181.85060452552, 181.61354452552, 181.30909452552, 180.93751452552, 180.49912452552, 179.99429452552, 179.42343452552, 178.78705452552, 178.08566452552, 177.31987452552, 176.49033452552, 175.59773452552, 174.64284452552, 173.62646452552, 172.54946452552, 171.41272452552, 170.21723452552, 168.96401452552, 167.65409452552, 166.28860452552, 164.86869452552, 163.39556452552, 161.87045452552, 160.29467452552, 158.66952452552, 156.99641452552, 155.27674452552, 153.51197452552, 151.70358452552, 149.85312452552, 147.96214452552, 146.03224452552, 144.06506452552, 142.06227452552, 140.02554452552, 137.95663452552, 135.85726452552, 133.72922452552, 131.57430452552, 129.39435452552, 127.19117452552, 124.96667452552, 122.72270452552, 120.46117452552, 118.18400452552, 115.89311452552, 113.59044452552, 111.27793452552, 108.95754452552, 106.63124452552, 104.30100452552, 101.96879452552, 99.636584525524, 97.306334525524, 94.980044525524, 92.659664525524, 90.347154525524, 88.044484525524, 85.753584525524, 83.476414525524, 81.214884525524, 78.970924525524, 76.746414525524, 74.543244525524, 72.363284525524, 70.208374525524, 68.080334525524, 65.980964525524, 63.912034525524, 61.875324525524, 59.872534525524, 57.905354525524, 55.975454525524, 54.084474525524, 52.234004525524, 50.425614525524, 48.660854525524, 46.941174525524, 45.268064525524, 43.642924525524, 42.067144525524, 40.542034525524, 39.068904525524, 37.648984525524, 36.283494525524, 34.973574525524, 33.720354525524, 32.524864525524, 31.388134525524, 30.311124525524, 29.294744525524, 28.339854525524, 27.447254525524, 26.617714525524, 25.851934525524, 25.150544525524, 24.514154525524, 23.943304525524, 23.438464525524, 23.000074525524, 22.628494525524, 22.324054525524, 22.086984525524, 21.917514525524, 21.899884525524 },
    y = { 102.30266872032, 100.07798872032, 97.748598720324, 95.423158720324, 93.103618720324, 90.791958720324, 88.490128720324, 86.200068720324, 83.923728720324, 81.663028720324, 79.419888720324, 77.196188720324, 74.993828720324, 72.814668720324, 70.660538720324, 68.533278720324, 66.434678720324, 64.366508720324, 62.330538720324, 60.328478720324, 58.362018720324, 56.432828720324, 54.542538720324, 52.692748720324, 50.885028720324, 49.120898720324, 47.401848720324, 45.729348720324, 44.104808720324, 42.529598720324, 41.005048720324, 39.532458720324, 38.113058720324, 36.748068720324, 35.438628720324, 34.185858720324, 32.990808720324, 31.854498720324, 30.777878720324, 29.761868720324, 28.807328720324, 27.915058720324, 27.085818720324, 26.320318720324, 25.619188720324, 24.983028720324, 24.412388720324, 23.907738720324, 23.469498720324, 23.098058720324, 22.793728720324, 22.556748720324, 22.387338720324, 22.285638720324, 22.251728720324, 22.285638720324, 22.387338720324, 22.556748720324, 22.793728720324, 23.098058720324, 23.469498720324, 23.907738720324, 24.412388720324, 24.983028720324, 25.619188720324, 26.320318720324, 27.085818720324, 27.915058720324, 28.807328720324, 29.761868720324, 30.777878720324, 31.854498720324, 32.990808720324, 34.185858720324, 35.438628720324, 36.748068720324, 38.113058720324, 39.532458720324, 41.005048720324, 42.529598720324, 44.104808720324, 45.729348720324, 47.401848720324, 49.120898720324, 50.885028720324, 52.692748720324, 54.542538720324, 56.432828720324, 58.362018720324, 60.328478720324, 62.330538720324, 64.366508720324, 66.434678720324, 68.533278720324, 70.660538720324, 72.814668720324, 74.993828720324, 77.196188720324, 79.419888720324, 81.663028720324, 83.923728720324, 86.200068720324, 88.490128720324, 90.791958720324, 93.103618720324, 95.423158720324, 97.748598720324, 100.07798872032, 101.92012872032, 102.30174872032, 103.05211872032, 103.80058872032, 104.54650872032, 105.28926872032, 106.02821872032, 106.76274872032, 107.49223872032, 108.21604872032, 108.93358872032, 109.64424872032, 110.34742872032, 111.04251872032, 111.72893872032, 112.40611872032, 113.07346872032, 113.73043872032, 114.37646872032, 115.01099872032, 115.63350872032, 116.24346872032, 116.84035872032, 117.42367872032, 117.99292872032, 118.54762872032, 119.08730872032, 119.61151872032, 120.11980872032, 120.61174872032, 121.08691872032, 121.54493872032, 121.98538872032, 122.40791872032, 122.81215872032, 123.19778872032, 123.56444872032, 123.91184872032, 124.23969872032, 124.54770872032, 124.83562872032, 125.10319872032, 125.35021872032, 125.57645872032, 125.78172872032, 125.96586872032, 126.12870872032, 126.27011872032, 126.38996872032, 126.48817872032, 126.56463872032, 126.61930872032, 126.65212872032, 126.66306872032, 126.65212872032, 126.61930872032, 126.56463872032, 126.48817872032, 126.38996872032, 126.27011872032, 126.12870872032, 125.96586872032, 125.78172872032, 125.57645872032, 125.35021872032, 125.10319872032, 124.83562872032, 124.54770872032, 124.23969872032, 123.91184872032, 123.56444872032, 123.19778872032, 122.81215872032, 122.40791872032, 121.98538872032, 121.54493872032, 121.08691872032, 120.61174872032, 120.11980872032, 119.61151872032, 119.08730872032, 118.54762872032, 117.99292872032, 117.42367872032, 116.84035872032, 116.24346872032, 115.63350872032, 115.01099872032, 114.37646872032, 113.73043872032, 113.07346872032, 112.40611872032, 111.72893872032, 111.04251872032, 110.34742872032, 109.64424872032, 108.93358872032, 108.21604872032, 107.49223872032, 106.76274872032, 106.02821872032, 105.28926872032, 104.54650872032, 103.80058872032, 103.05211872032, 102.30174872032, 102.17148872032 },
    pressure = {},
    tool = "pen",
    color = 16711680,
    width = 2.26,
    fill = 0,
    lineStyle = "plain",
  },
  stroke9 = {
    x = { 80.250154525524, 80.130814525524, 79.967794525524, 79.823624525524, 79.698434525524, 79.592324525524, 79.505384525524, 79.437684525524, 79.389284525524, 79.360234525524, 79.350544525524, 79.360234525524, 79.389284525524, 79.437684525524, 79.505384525524, 79.592324525524, 79.698434525524, 79.823624525524, 79.967794525524, 80.130814525524, 80.312544525524, 80.512844525524, 80.731534525524, 80.968424525524, 81.223324525524, 81.496014525524, 81.786264525524, 82.093824525524, 82.418444525524, 82.759844525524, 83.117734525524, 83.491804525524, 83.881754525524, 84.287234525524, 84.707924525524, 85.143454525524, 85.593444525524, 86.057544525524, 86.535334525524, 87.026424525524, 87.530394525524, 88.046814525524, 88.575254525524, 89.115264525524, 89.666394525524, 90.228164525524, 90.800104525524, 91.381734525524, 91.972554525524, 92.572074525524, 93.179784525524, 93.795164525524, 94.417704525524, 95.046864525524, 95.682114525524, 96.322934525524, 96.968764525524, 97.619054525524, 98.273274525524, 98.930844525524, 99.591234525524, 100.25387452552, 100.91819452552, 100.95113452552 },
    y = { 124.58207872032, 123.10911872032, 120.84841872032, 118.57207872032, 116.28201872032, 113.98018872032, 111.66852872032, 109.34898872032, 107.02354872032, 104.69415872032, 102.36279872032, 100.03142872032, 97.702038720324, 95.376598720324, 93.057058720324, 90.745398720324, 88.443568720324, 86.153508720324, 83.877168720324, 81.616478720324, 79.373328720324, 77.149628720324, 74.947268720324, 72.768108720324, 70.613978720324, 68.486718720324, 66.388118720324, 64.319958720324, 62.283978720324, 60.281918720324, 58.315458720324, 56.386268720324, 54.495978720324, 52.646188720324, 50.838468720324, 49.074338720324, 47.355298720324, 45.682798720324, 44.058248720324, 42.483038720324, 40.958488720324, 39.485898720324, 38.066508720324, 36.701508720324, 35.392078720324, 34.139298720324, 32.944258720324, 31.807938720324, 30.731328720324, 29.715318720324, 28.760768720324, 27.868508720324, 27.039268720324, 26.273758720324, 25.572628720324, 24.936478720324, 24.365828720324, 23.861178720324, 23.422938720324, 23.051498720324, 22.747168720324, 22.510188720324, 22.340778720324, 22.335748720324 },
    pressure = {},
    tool = "pen",
    color = 14687012,
    width = 2.26,
    fill = 0,
    lineStyle = "plain",
  },
}
return strokesData   -- Return the strokesData table