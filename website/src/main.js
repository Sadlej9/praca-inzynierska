import { createApp } from 'vue'
import App from './App.vue'
import PrimeVue from 'primevue/config';
import Card from 'primevue/card';
import Button from 'primevue/button';
import InputText from 'primevue/inputtext';
import ToastService from 'primevue/toastservice';
import ProgressSpinner from 'primevue/progressspinner';

import 'primevue/resources/themes/lara-light-blue/theme.css'       //theme
import 'primevue/resources/primevue.min.css'                 //core css
import 'primeicons/primeicons.css' 

const app = createApp(App);
app.use(PrimeVue);
app.use(ToastService);

app.component('Card', Card);
app.component('Button', Button);
app.component('InputText', InputText);
app.component('ProgressSpinner', ProgressSpinner);

app.mount('#app')
