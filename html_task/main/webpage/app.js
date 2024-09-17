// script.js

/**
 * Add gobals here
 */
var seconds = null;
var otaTimerVar = null;
var wifiConnectInterval = null;

function toogle_led() {
  //activar función de efecto visual en boton
  $.ajax({
    url: "/toogle_led.json",
    dataType: "json",
    method: "POST",
    cache: false,
    success: function (response) {
      alert("Led toggled successfully");
      // Aquí puedes agregar la lógica para manejar la respuesta exitosa
    },
    error: function (xhr, status, error) {
      alert("Error toggling led: " + error);
      // Aquí puedes agregar la lógica para manejar errores
    },
  });
}

// Nueva funcionalidad: Recibir temperatura
function recibirTemperatura() {
  // enviar un request
  $.ajax({
    url: "/get_temperature.json",
    dataType: "json",
    method: "POST",
    cache: false,
    success: function (response) {
      alert("Temperatura recibida: " + response.temperature);
      // escribir el dato en el cuadro de texto de la clase received-data
      document.getElementById("dataReceived").value = response.temperature;
    },
    error: function (xhr, status, error) {
      alert("Error al recibir la temperatura: " + error);
      // Aquí puedes agregar la lógica para manejar errores
    },
  });
}

document.addEventListener("DOMContentLoaded", () => {
  const checkboxes = document.querySelectorAll(
    ".checkbox-container input[type='checkbox']"
  );

  checkboxes.forEach((checkbox) => {
    checkbox.addEventListener("change", () => {
      if (checkbox.checked) {
        checkboxes.forEach((cb) => {
          if (cb !== checkbox) {
            cb.disabled = true;
          }
        });
      } else {
        checkboxes.forEach((cb) => {
          cb.disabled = false;
        });
      }
    });
  });
});

// Nueva funcionalidad: Flecha arriba
function flechaArriba() {
  // revisar qué checkbox de la clase "checkbox-container" está seleccionado dependiendo de la id
  var id = null;
  var checkboxes = document.querySelectorAll(
    ".checkbox-container input[type='checkbox']"
  );
  checkboxes.forEach((checkbox) => {
    if (checkbox.checked) {
      id = checkbox.id;
    }
  });
  if (id) {
    alert("Flecha arriba presionada para " + id);
    // Aquí puedes agregar la lógica para la flecha hacia arriba
  } else {
    alert("No hay checkbox seleccionado");
  }
}

// Nueva funcionalidad: Flecha abajo
function flechaAbajo() {
  // revisar qué checkbox de la clase "checkbox-container" está seleccionado dependiendo de la id
  var id = null;
  var checkboxes = document.querySelectorAll(
    ".checkbox-container input[type='checkbox']"
  );
  checkboxes.forEach((checkbox) => {
    if (checkbox.checked) {
      id = checkbox.id;
    }
  });
  if (id) {
    alert("Flecha abajo presionada para " + id);
    // Aquí puedes agregar la lógica para la flecha hacia abajo
  } else {
    alert("No hay checkbox seleccionado");
  }
}

// Nueva funcionalidad: Enviar datos
function enviarDatos() {
  const data = document.getElementById("dataReceived").value;
  alert("Datos enviados: " + data);
  // Aquí puedes agregar la lógica para enviar los datos
}

// función para efecto visual en boton
function buttonEffect(button) {
  button.classList.add("button-glow");
  setTimeout(() => {
    button.classList.remove("button-glow");
  }, 1000);
}

// Asignar el evento de clic a los botones
document.querySelectorAll(".buttons button").forEach((button) => {
  button.addEventListener("click", () => buttonEffect(button));
});

// función para efecto visual en flechas
function arrowEffect(arrow) {
  arrow.classList.add("arrow-glow");
  setTimeout(() => {
    arrow.classList.remove("arrow-glow");
  }, 1000);
}

// Asignar el evento de clic a las flechas
document.querySelectorAll(".arrow").forEach((arrow) => {
  arrow.addEventListener("click", () => arrowEffect(arrow));
});
