local strokesData = {
  [1] = {
    x = { 175.04924107455, 174.9969694731, 174.84023839903, 174.57929904308, 174.21453417214, 173.74651793651, 173.17597998557, 172.50379350623, 171.73098718447, 170.8587571668, 169.88845509872, 168.82156420189, 167.65973515846, 166.40477414962, 165.05859500975, 163.62327903373, 162.10105105403, 160.49424355629, 158.80534452522, 157.03694959867, 155.19178599065, 153.27270052979, 151.28265965936, 149.22472551439, 147.10206788305, 144.91797616819, 142.67578761849, 140.37899498171, 138.0311149285, 135.63577178272, 133.19666163702, 130.71755235282, 128.20225963741, 125.65465900541, 123.07868577877, 120.47829920234, 117.85753028979, 115.2203980933, 112.57099343387, 109.91337124809, 107.25165824131, 104.58993327308, 101.93231108729, 99.282906427859, 96.645774231377, 94.025005318825, 91.424618742397, 88.84864551575, 86.301044883754, 83.785752168348, 81.306642884147, 78.867532738442, 76.472189592664, 74.124309539456, 71.827516902669, 69.585328352974, 67.401236638109, 65.278579006772, 63.220644861799, 61.230603991377, 59.311518530513, 57.466366883955, 55.697971957411, 54.009060964871, 52.402253467137, 50.880025487437, 49.444709511418, 48.098530371545, 46.843569362704, 45.681740319271, 44.614849422442, 43.644547354367, 42.772317336689, 41.999511014937, 41.327324535598, 40.756786584649, 40.288770349025, 39.924017439547, 39.663066122136, 39.50633504806, 39.454075408081, 39.50633504806, 39.663066122136, 39.924017439547, 40.288770349025, 40.756786584649, 41.327324535598, 41.999511014937, 42.772317336689, 43.644547354367, 44.614849422442, 45.681740319271, 46.843569362704, 48.098530371545, 49.444709511418, 50.880025487437, 52.402253467137, 54.009060964871, 55.697971957411, 57.466366883955, 59.311518530513, 61.230603991377, 63.220644861799, 65.278579006772, 67.401236638109, 69.585328352974, 71.827516902669, 74.124309539456, 76.472189592664, 78.867532738442, 81.306642884147, 83.785752168348, 86.301044883754, 88.84864551575, 91.424618742397, 94.025005318825, 96.645774231377, 99.282906427859, 101.93231108729, 104.58993327308, 107.25165824131, 109.91337124809, 112.57099343387, 115.2203980933, 117.85753028979, 120.47829920234, 123.07868577877, 125.65465900541, 128.20225963741, 130.71755235282, 133.19666163702, 135.63577178272, 138.0311149285, 140.37899498171, 142.67578761849, 144.91797616819, 147.10206788305, 149.22472551439, 151.28265965936, 153.27270052979, 155.19178599065, 157.03694959867, 158.80534452522, 160.49424355629, 162.10105105403, 163.62327903373, 165.05859500975, 166.40477414962, 167.65973515846, 168.82156420189, 169.88845509872, 170.8587571668, 171.73098718447, 172.50379350623, 173.17597998557, 173.74651793651, 174.21453417214, 174.57929904308, 174.84023839903, 174.9969694731, 175.04924107455 },
    y = { 181.58120939333, 182.23745124001, 182.89267636217, 183.54587999678, 184.19605738077, 184.84220375111, 185.4833263062, 186.11842028301, 186.74652876434, 187.36667091006, 187.97788980301, 188.57924048744, 189.16980193056, 189.74866506106, 190.31494473052, 190.86774382908, 191.4062250542, 191.9295630648, 192.43694448129, 192.92757984696, 193.40072755101, 193.85564598261, 194.2916413768, 194.70803193009, 195.10418368482, 195.47948660631, 195.83336654423, 196.16526130973, 196.47466852129, 196.76110972033, 197.02415429409, 197.26337162985, 197.47842680659, 197.66896098036, 197.83468707601, 197.97535390278, 198.09073419283, 198.18066048566, 198.24500120515, 198.28363673665, 198.29653119577, 198.28363673665, 198.24500120515, 198.18066048566, 198.09073419283, 197.97535390278, 197.83468707601, 197.66896098036, 197.47842680659, 197.26337162985, 197.02415429409, 196.76110972033, 196.47466852129, 196.16526130973, 195.83336654423, 195.47948660631, 195.10418368482, 194.70803193009, 194.2916413768, 193.85564598261, 193.40072755101, 192.92757984696, 192.43694448129, 191.9295630648, 191.4062250542, 190.86774382908, 190.31494473052, 189.74866506106, 189.16980193056, 188.57924048744, 187.97788980301, 187.36667091006, 186.74652876434, 186.11842028301, 185.4833263062, 184.84220375111, 184.19605738077, 183.54587999678, 182.89267636217, 182.23745124001, 181.58120939333, 180.92496754665, 180.26974242448, 179.61653878988, 178.96636140588, 178.32021503555, 177.67909248045, 177.04399850365, 176.41589002232, 175.79575983806, 175.18454094511, 174.58317829922, 173.99261685609, 173.4137537256, 172.8474860176, 172.29467495758, 171.75619373246, 171.23285572185, 170.72547430537, 170.23483893969, 169.76169123564, 169.30677280404, 168.87077740985, 168.45438685657, 168.05823510183, 167.68293218035, 167.32906420389, 166.99715747693, 166.68775026536, 166.40130906633, 166.13827645403, 165.89904715681, 165.68400394153, 165.49346976776, 165.32774367211, 165.18707684534, 165.07168459382, 164.981758301, 164.91741758151, 164.87878205001, 164.86588759089, 164.87878205001, 164.91741758151, 164.981758301, 165.07168459382, 165.18707684534, 165.32774367211, 165.49346976776, 165.68400394153, 165.89904715681, 166.13827645403, 166.40130906633, 166.68775026536, 166.99715747693, 167.32906420389, 167.68293218035, 168.05823510183, 168.45438685657, 168.87077740985, 169.30677280404, 169.76169123564, 170.23483893969, 170.72547430537, 171.23285572185, 171.75619373246, 172.29467495758, 172.8474860176, 173.4137537256, 173.99261685609, 174.58317829922, 175.18454094511, 175.79575983806, 176.41589002232, 177.04399850365, 177.67909248045, 178.32021503555, 178.96636140588, 179.61653878988, 180.26974242448, 180.92496754665, 181.58120939333 },
    pressure = {},
    tool = "pen",
    color = 10820909,
    width = 1.41,
    fill = 0,
    lineStyle = "dot",
  },
  [2] = {
    x = { 60.574649219854, 60.574649219854 },
    y = { 47.085376573895, 178.58882532919 },
    pressure = {},
    tool = "pen",
    color = 3355596,
    width = 1.41,
    fill = 0,
    lineStyle = "dot",
  },
  [3] = {
    x = { 153.9294088736, 153.9294088736 },
    y = { 48.545264617657, 180.04871337296 },
    pressure = {},
    tool = "pen",
    color = 3355596,
    width = 1.41,
    fill = 0,
    lineStyle = "dot",
  },
  [4] = {
    x = { 39.696725684255, 39.696725684255, 39.819306776153, 39.948562365443, 40.31331527492, 40.781331510544, 41.351869461493, 42.024055940832, 42.796862262584, 43.669092280262, 44.639394348337, 45.706285245166, 46.868114288599, 48.12307529744, 49.469254437313, 50.904570413332, 52.426798393032, 54.033605890766, 55.722516883307, 57.49091180985, 59.336063456408, 61.255148917272, 63.245189787694, 65.303123932667, 67.425781564005, 69.609873278869, 71.852061828564, 74.148854465351, 76.496734518559, 78.892077664337, 81.331187810042, 83.810297094243, 86.325589809649, 88.873190441645, 91.449163668292, 94.04955024472, 96.670319157272, 99.307451353754, 101.95685601318, 104.61447819897, 107.27620316721, 109.93791617398, 112.59553835977, 115.2449430192, 117.88207521568, 120.50284412823, 123.10323070466, 125.67920393131, 128.2268045633, 130.74209727871, 133.22120656291, 135.66031670862, 138.05565985439, 140.4035399076, 142.70033254439, 144.94252109408, 147.12661280895, 149.24927044029, 151.30720458526, 153.29724545568, 155.21633091655, 157.06149452457, 158.82988945111, 160.51878848219, 162.12559597992, 163.64782395962, 165.08313993564, 166.42931907551, 167.68428008436, 168.84610912779, 169.91300002462, 170.88330209269, 171.75553211037, 172.52833843212, 173.20052491146, 173.77106286241, 174.23907909803, 174.60384396898, 174.86478332492, 175.021514399, 175.03744707019, 175.29189135072, 175.29189135072, 175.28364990143, 175.23137829999, 175.07464722591, 174.81370786997, 174.44894299902, 173.9809267634, 173.41038881245, 172.73820233311, 171.96539601136, 171.09316599368, 170.12286392561, 169.05597302878, 167.89414398534, 166.6391829765, 165.29300383663, 163.85768786061, 162.33545988091, 160.72865238318, 159.0397533521, 157.27135842556, 155.42619481754, 153.50710935667, 151.51706848625, 149.45913434128, 147.33647670994, 145.15238499507, 142.91019644538, 140.61340380859, 138.26552375538, 135.87018060961, 133.4310704639, 130.9519611797, 128.43666846429, 125.8890678323, 123.31309460565, 120.71270802922, 118.09193911667, 115.45480692019, 112.80540226076, 110.14778007497, 107.4860670682, 104.82434209996, 102.16671991417, 99.517315254744, 96.880183058262, 94.25941414571, 91.659027569282, 89.083054342635, 86.535453710639, 84.020160995233, 81.541051711032, 79.101941565327, 76.706598419549, 74.35871836634, 72.061925729554, 69.819737179859, 67.635645464994, 65.512987833657, 63.455053688683, 61.465012818262, 59.545927357397, 57.70077571084, 55.932380784296, 54.243469791755, 52.636662294022, 51.114434314322, 49.679118338303, 48.33293919843, 47.077978189588, 45.916149146156, 44.849258249327, 43.878956181252, 43.006726163574, 42.233919841822, 41.561733362482, 40.991195411533, 40.523179175909, 40.158426266432, 39.89747494902, 39.740743874944, 39.688484234966, 39.740743874944, 39.89747494902, 40.158426266432 },
    y = { 182.02677395941, 46.197534387072, 44.841977458748, 44.518419834253, 43.868242450258, 43.222096079921, 42.580973524826, 41.945879548021, 41.317771066695, 40.697640882431, 40.086421989489, 39.485059343593, 38.894497900466, 38.315634769974, 37.74936706198, 37.196556001955, 36.658074776834, 36.134736766227, 35.627355349745, 35.136719984069, 34.66357228002, 34.208653848419, 33.772658454228, 33.356267900945, 32.960116146207, 32.584813224723, 32.230945248269, 31.899038521301, 31.589631309736, 31.303190110706, 31.040157498408, 30.800928201184, 30.58588498591, 30.395350812136, 30.229624716484, 30.088957889716, 29.9735656382, 29.883639345374, 29.819298625886, 29.780663094384, 29.767768635264, 29.780663094384, 29.819298625886, 29.883639345374, 29.9735656382, 30.088957889716, 30.229624716484, 30.395350812136, 30.58588498591, 30.800928201184, 31.040157498408, 31.303190110706, 31.589631309736, 31.899038521301, 32.230945248269, 32.584813224723, 32.960116146207, 33.356267900945, 33.772658454228, 34.208653848419, 34.66357228002, 35.136719984069, 35.627355349745, 36.134736766227, 36.658074776834, 37.196556001955, 37.74936706198, 38.315634769974, 38.894497900466, 39.485059343593, 40.086421989489, 40.697640882431, 41.317771066695, 41.945879548021, 42.580973524826, 43.222096079921, 43.868242450258, 44.518419834253, 45.171623468857, 45.826848591024, 46.026820360573, 46.197534387072, 182.02677395941, 181.32492304721, 181.98116489389, 182.63639001606, 183.28959365066, 183.93977103466, 184.58591740499, 185.22703996009, 185.86213393689, 186.49024241822, 187.11038456395, 187.72160345689, 188.32295414132, 188.91351558445, 189.49237871494, 190.0586583844, 190.61145748296, 191.14993870808, 191.67327671869, 192.18065813517, 192.67129350085, 193.1444412049, 193.5993596365, 194.03535503069, 194.45174558397, 194.84789733871, 195.22320026019, 195.57708019811, 195.90897496361, 196.21838217518, 196.50482337421, 196.76786794797, 197.00708528373, 197.22214046047, 197.41267463424, 197.5784007299, 197.71906755666, 197.83444784671, 197.92437413954, 197.98871485903, 198.02735039053, 198.04024484965, 198.02735039053, 197.98871485903, 197.92437413954, 197.83444784671, 197.71906755666, 197.5784007299, 197.41267463424, 197.22214046047, 197.00708528373, 196.76786794797, 196.50482337421, 196.21838217518, 195.90897496361, 195.57708019811, 195.22320026019, 194.84789733871, 194.45174558397, 194.03535503069, 193.5993596365, 193.1444412049, 192.67129350085, 192.18065813517, 191.67327671869, 191.14993870808, 190.61145748296, 190.0586583844, 189.49237871494, 188.91351558445, 188.32295414132, 187.72160345689, 187.11038456395, 186.49024241822, 185.86213393689, 185.22703996009, 184.58591740499, 183.93977103466, 183.28959365066, 182.63639001606, 181.98116489389, 181.32492304721, 180.66868120053, 180.01345607837, 179.36025244376 },
    pressure = {},
    tool = "pen",
    color = 16711680,
    width = 2.26,
    fill = 0,
    lineStyle = "plain",
  },
  [5] = {
    x = { 175.29189135072, 175.23961974928, 175.0828886752, 174.82194931925, 174.45718444831, 173.98916821269, 173.41863026174, 172.7464437824, 171.97363746065, 171.10140744297, 170.1311053749, 169.06421447807, 167.90238543463, 166.64742442579, 165.30124528592, 163.8659293099, 162.3437013302, 160.73689383247, 159.04799480139, 157.27959987485, 155.43443626682, 153.51535080596, 151.52530993554, 149.46737579057, 147.34471815923, 145.16062644436, 142.91843789467, 140.62164525788, 138.27376520467, 135.8784220589, 133.43931191319, 130.96020262899, 128.44490991358, 125.89730928159, 123.32133605494, 120.72094947851, 118.10018056596, 115.46304836948, 112.81364371005, 110.15602152426, 107.49430851749, 104.83258354925, 102.17496136346, 99.525556704033, 96.888424507551, 94.267655594999, 91.667269018571, 89.091295791924, 86.543695159928, 84.028402444522, 81.549293160321, 79.110183014616, 76.714839868838, 74.36695981563, 72.070167178843, 69.827978629148, 67.643886914283, 65.521229282946, 63.463295137973, 61.473254267551, 59.554168806687, 57.709017160129, 55.940622233586, 54.251711241045, 52.644903743311, 51.122675763611, 49.687359787592, 48.341180647719, 47.086219638878, 45.924390595445, 44.857499698616, 43.887197630541, 43.014967612863, 42.242161291111, 41.569974811772, 40.999436860823, 40.531420625199, 40.166667715722, 39.90571639831, 39.748985324234, 39.696725684255, 39.748985324234, 39.90571639831, 40.166667715722, 40.531420625199, 40.999436860823, 41.569974811772, 42.242161291111, 43.014967612863, 43.887197630541, 44.857499698616, 45.924390595445, 47.086219638878, 48.341180647719, 49.687359787592, 51.122675763611, 52.644903743311, 54.251711241045, 55.940622233586, 57.709017160129, 59.554168806687, 61.473254267551, 63.463295137973, 65.521229282946, 67.643886914283, 69.827978629148, 72.070167178843, 74.36695981563, 76.714839868838, 79.110183014616, 81.549293160321, 84.028402444522, 86.543695159928, 89.091295791924, 91.667269018571, 94.267655594999, 96.888424507551, 99.525556704033, 102.17496136346, 104.83258354925, 107.49430851749, 110.15602152426, 112.81364371005, 115.46304836948, 118.10018056596, 120.72094947851, 123.32133605494, 125.89730928159, 128.44490991358, 130.96020262899, 133.43931191319, 135.8784220589, 138.27376520467, 140.62164525788, 142.91843789467, 145.16062644436, 147.34471815923, 149.46737579057, 151.52530993554, 153.51535080596, 155.43443626682, 157.27959987485, 159.04799480139, 160.73689383247, 162.3437013302, 163.8659293099, 165.30124528592, 166.64742442579, 167.90238543463, 169.06421447807, 170.1311053749, 171.10140744297, 171.97363746065, 172.7464437824, 173.41863026174, 173.98916821269, 174.45718444831, 174.82194931925, 175.0828886752, 175.23961974928, 175.29189135072 },
    y = { 46.431608292941, 47.08785013962, 47.743075261787, 48.396278896391, 49.046456280386, 49.692602650723, 50.333725205818, 50.968819182623, 51.596927663949, 52.217069809678, 52.82828870262, 53.429639387051, 54.020200830178, 54.59906396067, 55.165343630129, 55.718142728689, 56.25662395381, 56.779961964417, 57.287343380899, 57.777978746575, 58.251126450625, 58.706044882225, 59.142040276416, 59.558430829699, 59.954582584437, 60.329885505921, 60.68376544384, 61.015660209343, 61.325067420908, 61.611508619939, 61.874553193701, 62.11377052946, 62.328825706199, 62.519359879973, 62.685085975625, 62.825752802393, 62.941133092444, 63.03105938527, 63.095400104758, 63.13403563626, 63.146930095381, 63.13403563626, 63.095400104758, 63.03105938527, 62.941133092444, 62.825752802393, 62.685085975625, 62.519359879973, 62.328825706199, 62.11377052946, 61.874553193701, 61.611508619939, 61.325067420908, 61.015660209343, 60.68376544384, 60.329885505921, 59.954582584437, 59.558430829699, 59.142040276416, 58.706044882225, 58.251126450625, 57.777978746575, 57.287343380899, 56.779961964417, 56.25662395381, 55.718142728689, 55.165343630129, 54.59906396067, 54.020200830178, 53.429639387051, 52.82828870262, 52.217069809678, 51.596927663949, 50.968819182623, 50.333725205818, 49.692602650723, 49.046456280386, 48.396278896391, 47.743075261787, 47.08785013962, 46.431608292941, 45.775366446261, 45.120141324095, 44.46693768949, 43.816760305495, 43.170613935158, 42.529491380063, 41.894397403258, 41.266288921932, 40.646158737669, 40.034939844727, 39.43357719883, 38.843015755704, 38.264152625211, 37.697884917217, 37.145073857193, 36.606592632071, 36.083254621465, 35.575873204983, 35.085237839306, 34.612090135257, 34.157171703656, 33.721176309466, 33.304785756183, 32.908634001445, 32.53333107996, 32.179463103507, 31.847556376538, 31.538149164974, 31.251707965943, 30.988675353645, 30.749446056422, 30.534402841147, 30.343868667373, 30.178142571721, 30.037475744954, 29.922083493438, 29.832157200611, 29.767816481123, 29.729180949621, 29.716286490501, 29.729180949621, 29.767816481123, 29.832157200611, 29.922083493438, 30.037475744954, 30.178142571721, 30.343868667373, 30.534402841147, 30.749446056422, 30.988675353645, 31.251707965943, 31.538149164974, 31.847556376538, 32.179463103507, 32.53333107996, 32.908634001445, 33.304785756183, 33.721176309466, 34.157171703656, 34.612090135257, 35.085237839306, 35.575873204983, 36.083254621465, 36.606592632071, 37.145073857193, 37.697884917217, 38.264152625211, 38.843015755704, 39.43357719883, 40.034939844727, 40.646158737669, 41.266288921932, 41.894397403258, 42.529491380063, 43.170613935158, 43.816760305495, 44.46693768949, 45.120141324095, 45.775366446261, 46.431608292941 },
    pressure = {},
    tool = "pen",
    color = 16711680,
    width = 2.26,
    fill = 0,
    lineStyle = "plain",
  },
  [6] = {
    x = { 153.9294088736, 153.89336897997, 153.78528518348, 153.60537279051, 153.3538471074, 153.03113874691, 152.63773812909, 152.17424332719, 151.64137202909, 151.0399376144, 150.37088503886, 149.63523102696, 148.83412387935, 147.96878366543, 147.04056203074, 146.05085846666, 145.00123992509, 143.89330924232, 142.7287529849, 141.50937733402, 140.23708416259, 138.9138231894, 137.54161590198, 136.1226273255, 134.65898660069, 133.15297836734, 131.60692314964, 130.02322520202, 128.40428877892, 126.75262578795, 125.07077205968, 123.3613471549, 121.62698259589, 119.87033382784, 118.09412806474, 116.30109252058, 114.49397833228, 112.67559644409, 110.84875780023, 109.01623746057, 107.18090617666, 105.34557489275, 103.51305455309, 101.68621590923, 99.867834021036, 98.060731794202, 96.26768428858, 94.491466564017, 92.734817795967, 91.000453236954, 89.291028332176, 87.609186565368, 85.957523574405, 84.338587151303, 82.754877242216, 81.208822024512, 79.702825752629, 78.239185027819, 76.820184489871, 75.447989163925, 74.124716229261, 72.8524350193, 71.633071329885, 70.468515072465, 69.360572428232, 68.310953886662, 67.321250322583, 66.393028687889, 65.527688473971, 64.726569364893, 63.990915353, 63.321862777454, 62.720440324233, 62.187569026132, 61.724074224226, 61.330661644941, 61.007953284451, 60.75645152428, 60.576515208373, 60.468443373352, 60.432403479725, 60.468443373352, 60.576515208373, 60.75645152428, 61.007953284451, 61.330661644941, 61.724074224226, 62.187569026132, 62.720440324233, 63.321862777454, 63.990915353, 64.726569364893, 65.527688473971, 66.393028687889, 67.321250322583, 68.310953886662, 69.360572428232, 70.468515072465, 71.633071329885, 72.8524350193, 74.124716229261, 75.447989163925, 76.820184489871, 78.239185027819, 79.702825752629, 81.208822024512, 82.754877242216, 84.338587151303, 85.957523574405, 87.609186565368, 89.291028332176, 91.000453236954, 92.734817795967, 94.491466564017, 96.26768428858, 98.060731794202, 99.867834021036, 101.68621590923, 103.51305455309, 105.34557489275, 107.18090617666, 109.01623746057, 110.84875780023, 112.67559644409, 114.49397833228, 116.30109252058, 118.09412806474, 119.87033382784, 121.62698259589, 123.3613471549, 125.07077205968, 126.75262578795, 128.40428877892, 130.02322520202, 131.60692314964, 133.15297836734, 134.65898660069, 136.1226273255, 137.54161590198, 138.9138231894, 140.23708416259, 141.50937733402, 142.7287529849, 143.89330924232, 145.00123992509, 146.05085846666, 147.04056203074, 147.96878366543, 148.83412387935, 149.63523102696, 150.37088503886, 151.0399376144, 151.64137202909, 152.17424332719, 152.63773812909, 153.03113874691, 153.3538471074, 153.60537279051, 153.78528518348, 153.89336897997, 153.9294088736 },
    y = { 45.081862441941, 45.351211996313, 45.620140954879, 45.888252755879, 46.155114786482, 46.420318467907, 46.68346723839, 46.944140502129, 47.201941697362, 47.456474262329, 47.707353652291, 47.954171288467, 48.196566660166, 48.434155222648, 48.666588482248, 48.893481894227, 49.114498981941, 49.329303268743, 49.537546260968, 49.738927533038, 49.933134642353, 50.119855146317, 50.298800636375, 50.46970673802, 50.632309076746, 50.78635529507, 50.931593035506, 51.067818008664, 51.194813908129, 51.312388461532, 51.420349396504, 51.518540491748, 51.606805525962, 51.685012311892, 51.753028662287, 51.810770457984, 51.858129545776, 51.895033823526, 51.921447240163, 51.93729769355, 51.942597200711, 51.93729769355, 51.921447240163, 51.895033823526, 51.858129545776, 51.810770457984, 51.753028662287, 51.685012311892, 51.606805525962, 51.518540491748, 51.420349396504, 51.312388461532, 51.194813908129, 51.067818008664, 50.931593035506, 50.78635529507, 50.632309076746, 50.46970673802, 50.298800636375, 50.119855146317, 49.933134642353, 49.738927533038, 49.537546260968, 49.329303268743, 49.114498981941, 48.893481894227, 48.666588482248, 48.434155222648, 48.196566660166, 47.954171288467, 47.707353652291, 47.456474262329, 47.201941697362, 46.944140502129, 46.68346723839, 46.420318467907, 46.155114786482, 45.888252755879, 45.620140954879, 45.351211996313, 45.081862441941, 44.812500870546, 44.543571911981, 44.275472128005, 44.008598080378, 43.743394398954, 43.480245628469, 43.21957236473, 42.961771169498, 42.707238604532, 42.456371231591, 42.209541578392, 41.967146206695, 41.729557644211, 41.497136401635, 41.270230972632, 41.049213884919, 40.834421615139, 40.626166605892, 40.424785333823, 40.230578224506, 40.043869737566, 39.864912230486, 39.69400612884, 39.531403790113, 39.377369588813, 39.232119831354, 39.095894858195, 38.968898958731, 38.851324405328, 38.743363470355, 38.645172375112, 38.556907340898, 38.478712571991, 38.410684204573, 38.352954425899, 38.305583321083, 38.268679043334, 38.242265626696, 38.226415173309, 38.22111566615, 38.226415173309, 38.242265626696, 38.268679043334, 38.305583321083, 38.352954425899, 38.410684204573, 38.478712571991, 38.556907340898, 38.645172375112, 38.743363470355, 38.851324405328, 38.968898958731, 39.095894858195, 39.232119831354, 39.377369588813, 39.531403790113, 39.69400612884, 39.864912230486, 40.043869737566, 40.230578224506, 40.424785333823, 40.626166605892, 40.834421615139, 41.049213884919, 41.270230972632, 41.497136401635, 41.729557644211, 41.967146206695, 42.209541578392, 42.456371231591, 42.707238604532, 42.961771169498, 43.21957236473, 43.480245628469, 43.743394398954, 44.008598080378, 44.275472128005, 44.543571911981, 44.812500870546, 45.081862441941 },
    pressure = {},
    tool = "pen",
    color = 3355596,
    width = 1.41,
    fill = 0,
    lineStyle = "dot",
  },
  [7] = {
    x = { 154.12000285469, 154.08396296107, 153.97589112605, 153.79595481014, 153.54445304997, 153.22173272801, 152.82832014873, 152.36484926975, 151.83196601019, 151.23054355696, 150.56149098142, 149.82583696953, 149.02471786045, 148.15937764653, 147.23114405037, 146.24145244776, 145.19183390619, 144.08389126195, 142.919346966, 141.69998327658, 140.42767814369, 139.10440520903, 137.73223380601, 136.31320934513, 134.84958058179, 133.34357234844, 131.7975290922, 130.21381918312, 128.59487079855, 126.94320780759, 125.26136604078, 123.551941136, 121.81758853845, 120.06092780894, 118.28472204584, 116.49167454022, 114.68456035192, 112.86619042518, 111.03933981987, 109.20683144167, 107.37150015776, 105.53615691239, 103.70364853419, 101.87679792887, 100.05841604067, 98.2513257753, 96.458278269678, 94.682060545115, 92.925411777066, 91.191047218052, 89.481622313275, 87.799780546467, 86.148117555504, 84.529181132401, 82.945471223314, 81.399416005611, 79.893419733727, 78.429779008917, 77.010778470969, 75.638583145023, 74.315310210359, 73.043029000398, 71.823665310984, 70.659109053563, 69.55116640933, 68.501535906296, 67.511844303681, 66.583610707523, 65.718282455069, 64.917163345991, 64.181509334098, 63.512456758552, 62.911022343867, 62.378151045765, 61.914668205324, 61.521255626039, 61.198547265549, 60.947045505378, 60.767109189471, 60.65903735445, 60.622997460823, 60.65903735445, 60.767109189471, 60.947045505378, 61.198547265549, 61.521255626039, 61.914668205324, 62.378151045765, 62.911022343867, 63.512456758552, 64.181509334098, 64.917163345991, 65.718282455069, 66.583610707523, 67.511844303681, 68.501535906296, 69.55116640933, 70.659109053563, 71.823665310984, 73.043029000398, 74.315310210359, 75.638583145023, 77.010778470969, 78.429779008917, 79.893419733727, 81.399416005611, 82.945471223314, 84.529181132401, 86.148117555504, 87.799780546467, 89.481622313275, 91.191047218052, 92.925411777066, 94.682060545115, 96.458278269678, 98.2513257753, 100.05841604067, 101.87679792887, 103.70364853419, 105.53615691239, 107.37150015776, 109.20683144167, 111.03933981987, 112.86619042518, 114.68456035192, 116.49167454022, 118.28472204584, 120.06092780894, 121.81758853845, 123.551941136, 125.26136604078, 126.94320780759, 128.59487079855, 130.21381918312, 131.7975290922, 133.34357234844, 134.84958058179, 136.31320934513, 137.73223380601, 139.10440520903, 140.42767814369, 141.69998327658, 142.919346966, 144.08389126195, 145.19183390619, 146.24145244776, 147.23114405037, 148.15937764653, 149.02471786045, 149.82583696953, 150.56149098142, 151.23054355696, 151.83196601019, 152.36484926975, 152.82832014873, 153.22173272801, 153.54445304997, 153.79595481014, 153.97589112605, 154.08396296107, 154.12000285469 },
    y = { 179.55983684173, 179.82919841313, 180.0981273717, 180.36622715567, 180.63308918627, 180.89830488471, 181.1614536552, 181.42212691894, 181.67992811417, 181.93446067914, 182.18532805208, 182.43215770528, 182.67455307698, 182.91214163946, 183.14456288204, 183.37145629402, 183.59247338173, 183.80727766854, 184.01553267778, 184.21691394985, 184.41110904215, 184.59782954611, 184.77678705318, 184.94769315483, 185.11029549356, 185.26432969486, 185.40957945232, 185.54580442547, 185.67280032494, 185.79037487834, 185.89833581331, 185.99652690856, 186.08479194277, 186.1629987287, 186.2310150791, 186.28875687479, 186.33610394556, 186.37302024034, 186.39943365697, 186.41528411036, 186.42058361752, 186.41528411036, 186.39943365697, 186.37302024034, 186.33610394556, 186.28875687479, 186.2310150791, 186.1629987287, 186.08479194277, 185.99652690856, 185.89833581331, 185.79037487834, 185.67280032494, 185.54580442547, 185.40957945232, 185.26432969486, 185.11029549356, 184.94769315483, 184.77678705318, 184.59782954611, 184.41110904215, 184.21691394985, 184.01553267778, 183.80727766854, 183.59247338173, 183.37145629402, 183.14456288204, 182.91214163946, 182.67455307698, 182.43215770528, 182.18532805208, 181.93446067914, 181.67992811417, 181.42212691894, 181.1614536552, 180.89830488471, 180.63308918627, 180.36622715567, 180.0981273717, 179.82919841313, 179.55983684173, 179.29048728736, 179.02155832879, 178.75344652779, 178.48658449718, 178.22138081576, 177.95823204528, 177.69755878154, 177.4397575863, 177.18522502134, 176.9343576484, 176.68752799521, 176.44513262351, 176.20754406102, 175.97512281844, 175.74821738944, 175.52720030173, 175.31239601493, 175.1041530227, 174.90277175064, 174.70856464132, 174.52184413735, 174.3428986473, 174.17199254565, 174.00939020692, 173.8553439886, 173.71010624817, 173.57386925799, 173.44688537555, 173.32931082213, 173.22134988716, 173.12315879193, 173.03489375771, 172.95668697177, 172.88867062138, 172.83092882569, 172.7835697379, 172.74666546015, 172.7202520435, 172.70440159012, 172.69910208296, 172.70440159012, 172.7202520435, 172.74666546015, 172.7835697379, 172.83092882569, 172.88867062138, 172.95668697177, 173.03489375771, 173.12315879193, 173.22134988716, 173.32931082213, 173.44688537555, 173.57386925799, 173.71010624817, 173.8553439886, 174.00939020692, 174.17199254565, 174.3428986473, 174.52184413735, 174.70856464132, 174.90277175064, 175.1041530227, 175.31239601493, 175.52720030173, 175.74821738944, 175.97512281844, 176.20754406102, 176.44513262351, 176.68752799521, 176.9343576484, 177.18522502134, 177.4397575863, 177.69755878154, 177.95823204528, 178.22138081576, 178.48658449718, 178.75344652779, 179.02155832879, 179.29048728736, 179.55983684173 },
    pressure = {},
    tool = "pen",
    color = 3355596,
    width = 1.41,
    fill = 0,
    lineStyle = "dot",
  },
}
return strokesData   -- Return the strokesData table