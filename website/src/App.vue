<!-- Struktura strony / ulozenie elementow na stronie [HTML] -->
<template>
  <div>
    <div v-if="ConnectState == 2" class="measurement-view-container">

      <div id="cards-container">

        <Card v-if="Readouts == 2" class="card-class">
          <template #title>
            Temperatura 1
          </template>
          <template #content>
            <span class="measurement">{{ Temperature1 }}&#8451</span>
          </template>
        </Card>

        <Card class="card-class">
          <template #title>
            {{ Readouts == 1 ? 'Temperatura' : 'Temperatura 2' }}
          </template>
          <template #content>
            <span class="measurement">{{ Temperature2 }}&#8451</span>
          </template>
        </Card>

        

        <Card class="card-class">
          <template #title>
            Wilgotność
          </template>
          <template #content>
            <span class="measurement">{{ Humidity }}%</span>
          </template>
        </Card>

      </div>

      <div class="button-row">
        <div>
          <Button class="p-button-secondary mra" label="10 sekund" @click="setRange(10)" />
          <Button class="p-button-secondary mra" outlined label="Minuta" @click="setRange(60)" />
          <Button class="p-button-secondary mra" outlined label="10 minut" @click="setRange(600)" />
          <Button class="p-button-secondary mra" outlined label="Godzina" @click="setRange(3600)" />
          <Button class="p-button-secondary mra" outlined label="24H" @click="setRange(3600 * 24)" />
        </div>
        
        <Button label="Pobierz dane" icon="pi pi-download" @click="exportToCSV()" />
      </div>

      <div class="button-row">

      </div>

      <div class="center-row">
        <Card class="chart-card">
          <template #content>
            <apexchart type="line" height="350" ref="chart" :options="TemperatureChartOptions"
              :series="TemperatureData" />
          </template>
        </Card>
      </div>
      <div class="center-row">
        <Card class="chart-card">
          <template #content>
            <apexchart type="line" height="350" ref="chart" :options="HumidityChartOptions" :series="HumidityData" />
          </template>
        </Card>
      </div>
    </div>



    <!-- Ekran łączenia -->

    <div v-if="ConnectState == 0 || ConnectState == 1" class="connect-screen-container">
      <Card class="connect-card">
        <template #title >
          Połącz z urządzeniem
        </template>
        <template #content >
          <span class="s1">Aby kontunuować podaj adres IP urządzenia pomiarowego:</span>
          <InputText class="ip-input" v-model="IPAddress"/>
        </template>
        <template #footer>
          <Button icon="pi pi-check" label="Połącz" @click="connect"/>
        </template>
      </Card>
    </div>

  </div>

</template>

<!-- Logika strony / działanie [JavaScript]-->
<script>
import { ref, watch } from 'vue'
import VueApexCharts from 'vue3-apexcharts'
import moment from 'moment'


export default {
  name: 'App',
  components: {
    'apexchart': VueApexCharts,
  },
  setup() {

    // deklaracja zmiennych
    const IPAddress = ref('');

    const Readouts = ref(2); //liczba odczytów (czy 1 czy 2 czujniki)
    const Temperature1 = ref(0);
    const Temperature2 = ref(0);
    const Humidity = ref(0);
    const ConnectState = ref(0);
    //7776
    const Range = ref(60000);
    const Measurements = ref([]);
    const TemperatureChartOptions = ref({
      chart: {
        id: 'realtime',
        height: 350,
        type: 'line',
        animations: {
          enabled: true,
          easing: 'linear',
          dynamicAnimation: {
            speed: 1000
          }
        },
        toolbar: {
          show: false
        },
        zoom: {
          enabled: false
        }
      },
      dataLabels: {
        enabled: false
      },
      stroke: {
        curve: 'smooth'
      },
      title: {
        text: 'Wykres temperatury',
        align: 'left'
      },
      markers: {
        size: 0
      },
      xaxis: {
        type: 'datetime',
        range: Range.value,
        labels:{
          datetimeUTC: false,
        }
      },
      yaxis: {
        min: 0,
        
      },
      legend: {
        show: true
      },
    })

    const HumidityChartOptions = ref({
      chart: {
        id: 'realtime',
        height: 350,
        type: 'line',
        animations: {
          enabled: true,
          easing: 'linear',
          dynamicAnimation: {
            speed: 500
          }
        },
        toolbar: {
          show: false
        },
        zoom: {
          enabled: false
        }
      },
      dataLabels: {
        enabled: false
      },
      stroke: {
        curve: 'smooth'
      },
      title: {
        text: 'Wykres wilgotności',
        align: 'left'
      },
      markers: {
        size: 0
      },
      xaxis: {
        type: 'datetime',
        range: Range.value,
        labels:{
          datetimeUTC: false,
        }
      },
      yaxis: {
        min: 0,
      },
      legend: {
        show: true
      },
    });

    

    const TemperatureData = ref([
      {
        name: "Temperatura2",
        data: []
      },
      {
        name: "Temperatura1",
        data: []
      }
    ]);

    const HumidityData = ref([
      {
        name: "Wilgotność",
        data: []
      },
    ]);




    //funkcja łącząca się z arduino i pobierająca z niego dane w formacie JSON
    function getCurrentMeasurements() {
      let xhr = new XMLHttpRequest();
      xhr.open('GET', "http://" + IPAddress.value, true);
      xhr.responseType = 'json';
      xhr.onload = function () {
        var status = xhr.status;
        if (status === 200) {
          updateVariables(xhr.response);
        } else {
          console.warn("Cannot get data from arduino", status, xhr.response);
        }
      };
      xhr.send();
    }

    // Dane pobrane jako JSON są przypisywane do konkretnych zmiennych
    function updateVariables(newMeasurements) {
      Temperature1.value = newMeasurements.t1;
      Temperature2.value = newMeasurements.t2;
      Humidity.value = newMeasurements.h;
      let time = moment(newMeasurements.timestamp, "DD/MM/YYYY hh:mm:ss").toDate();

      
      TemperatureData.value[0]['data'].push({ x: time, y: newMeasurements.t2 });
      if(Readouts.value == 2){
        TemperatureData.value[1]['data'].push({ x: time, y: newMeasurements.t1 });
      }

      HumidityData.value[0]['data'].push({ x: time, y: newMeasurements.h });

      // chartOptions.value['xaxis']['categories'].push(newMeasurements.timestamp);
      //cały obiekt JSON jest dodawany do tablicy ze wszystkimi odczytami
      Measurements.value.push(newMeasurements);
    }

    // setInterval(getCurrentMeasurements, 500);

    function genData() {
      let measurement = {
        t1: Math.floor(Math.random() * 25),
        t2: Math.floor(Math.random() * 25),
        h: Math.floor(Math.random() * 10),
        timestamp: Date.now(),
      }

      updateVariables(measurement);
    }

    function setRange(timeInSeconds){
      TemperatureChartOptions.value = {
      chart: {
        id: 'realtime',
        height: 350,
        type: 'line',
        animations: {
          enabled: true,
          easing: 'linear',
          dynamicAnimation: {
            speed: 1000
          }
        },
        toolbar: {
          show: false
        },
        zoom: {
          enabled: false
        }
      },
      dataLabels: {
        enabled: false
      },
      stroke: {
        curve: 'smooth'
      },
      title: {
        text: 'Wykres temperatury',
        align: 'left'
      },
      markers: {
        size: 0
      },
      xaxis: {
        type: 'datetime',
        range: timeInSeconds * 1000,
      },
      yaxis: {
        min: 0,
        forceNiceScale: true,
      },
      legend: {
        show: true
      },
    };

    HumidityChartOptions.value = {
      chart: {
        id: 'realtime',
        height: 350,
        type: 'line',
        animations: {
          enabled: true,
          easing: 'linear',
          dynamicAnimation: {
            speed: 1000
          }
        },
        toolbar: {
          show: false
        },
        zoom: {
          enabled: false
        }
      },
      dataLabels: {
        enabled: false
      },
      stroke: {
        curve: 'smooth'
      },
      title: {
        text: 'Wykres wilgotności',
        align: 'left'
      },
      markers: {
        size: 0
      },
      xaxis: {
        type: 'datetime',
        range: timeInSeconds * 1000,
      },
      yaxis: {
        min: 0,
        forceNiceScale: true,
      },
      legend: {
        show: true
      },
    };
    }
    // function exportToCSV(objArray) {
    //   var arr = typeof objArray !== 'object' ? JSON.parse(objArray) : objArray;
    //   var str =
    //     `${Object.keys(arr[0])
    //       .map((value) => `"${value}"`)
    //       .join(',')}` + '\r\n';
    //   var csvContent = arr.reduce((st, next) => {
    //     st +=
    //       `${Object.values(next)
    //         .map((value) => `"${value}"`)
    //         .join(',')}` + '\r\n';
    //     return st;
    //   }, str);
    //   var element = document.createElement('a');
    //   element.href = 'data:text/csv;charset=utf-8,' + encodeURI(csvContent);
    //   element.target = '_blank';
    //   element.download = 'export.csv';
    //   element.click();
    // }

    function exportToCSV() {
      let arr = Measurements.value;
      let measurementStrings = arr.map(measurement => {
        if(Readouts.value == 2){
          return "" + measurement.t1 + ";" + measurement.t2 + ";" + measurement.h + ";" + measurement.timestamp;
        }
        else{
          return "" + measurement.t2 + ";" + measurement.h + ";" + measurement.timestamp;
        }
        
      })
      let titles = [];
      if(Readouts.value == 2){
        titles = ["Temperatura 1;Temperatura 2;Wilgotność;Czas"];
      }
      else{
        titles = ["Temperatura;Wilgotność;Czas"];
      }
      let data = titles.concat(measurementStrings);

      // let csvContent = array.map(measurement => {
      //   return Object.values(it).toString()
      // }).join('\n')
      let csvContent = data.join('\n');

      var element = document.createElement('a');
      element.href = 'data:text/csv;charset=utf-8,' + encodeURI(csvContent);
      element.target = '_blank';
      element.download = 'Dane pomiarowe.csv';
      element.click();
    }

    function connect(){
      ConnectState.value = 1;
      let xhr = new XMLHttpRequest();
      xhr.open('GET', "http://" + IPAddress.value, true);
      xhr.responseType = 'json';
      xhr.onload = function () {
        var status = xhr.status;
        if (status === 200) {
          if(Object.hasOwn(xhr.response, 't1') &&
          Object.hasOwn(xhr.response, 't2') &&
          Object.hasOwn(xhr.response, 'h') &&
          Object.hasOwn(xhr.response, 'timestamp')){
            ConnectState.value = 2;
            setInterval(getCurrentMeasurements, 1000); //zmiana z 500 na 1000

          };
        } else {
          
        }
      };
      xhr.send();
    }

    // setInterval(genData, 500);

    if(Readouts.value == 1){
      TemperatureData.value.splice(1, 1);
      TemperatureData.value[0]['name'] = 'Temperatura';
    }

    return {
      ConnectState,
      Temperature1,
      Temperature2,
      Humidity,
      Measurements,
      TemperatureChartOptions,
      HumidityChartOptions,
      TemperatureData,
      HumidityData,
      IPAddress,
      Range,
      Readouts,
      exportToCSV,
      connect,
      setRange,
    }
  }
}
</script>

<style>
body {
  background-color: #eff3f8;
}

#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}

.measurement {
  font-size: 2rem;
  font-weight: 700;
  color: var(--primary-color);
}

#cards-container {
  display: flex;
  flex-direction: row;
  justify-content: center;
  flex-wrap: wrap;
}

.card-class {
  margin: 1rem;
  width: 15rem;
}

.button-row {
  display: flex;
  flex-direction: row;
  justify-content: space-between;
  margin-left: 10%;
  margin-right: 10%;
}

.chart-card {
  margin: 1rem;
  max-width: 1500px;
  width: 90%;
}

.center-row {
  display: flex;
  flex-direction: row;
  justify-content: center;
}

.measurement-view-container {}

.connect-screen-container {
  position: absolute;
  top: 0;
  left: 0;
  width: 100vw;
  height: 100vh;
  background-color: #eff3f8;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-content: center;
  justify-items: center;
  align-items: center;
}

.connect-card{
  width: 500px;
}

.connect-card .p-card-footer{
  text-align: right;
}

.s1{
  display: block;
  margin-bottom: 1rem;
  text-align: left;
}

.ip-input{
  display: block;
  text-align: center;
  width: 100%;
  margin: 0 auto;
}

.mra{
  margin-right: 10px!important;
  margin-bottom: 5px!important;
}
</style>
