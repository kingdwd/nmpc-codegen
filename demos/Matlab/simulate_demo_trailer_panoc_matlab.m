function [ state_history,time_history,iteration_history ] = simulate_demo_trailer_panoc_matlab( trailer_controller, simulator, ...
    initial_state,reference_state,reference_input )
%SIMULATE_DEMO_TRAILER_PANOC_MATLAB Summary of this function goes here
%   Detailed explanation goes here
    % -- simulate controller --
    simulation_time = 3;
    number_of_steps = ceil(simulation_time / trailer_controller.model.step_size);
    % setup a simulator to test
    
    %% forbes panoc specific stuff
    opt_zerofpr2.tol = 0.001;
    opt_zerofpr2.display = 0;
    opt_zerofpr2.maxit = 10000;
    opt_zerofpr2.adaptive = 1;
    opt_zerofpr2.solver = 'zerofpr2';
    opt_zerofpr2.method = 'lbfgs';
    opt_zerofpr2.linesearch = 'backtracking';
    opt_zerofpr2.report = 1;
    g = indBox(-4, 4);
    inputs = repmat(zeros(trailer_controller.model.number_of_inputs, 1), ...
        trailer_controller.horizon, 1);
    
    %%
    state = initial_state;
    state_history = zeros(trailer_controller.model.number_of_states, number_of_steps);
    time_history = zeros(number_of_steps,1);
    iteration_history = zeros(number_of_steps,1);
    
    for i=1:number_of_steps
        cost = @(inputs_horizon) simulator.evaluate_cost_gradient(...
            state,reference_state,reference_input,inputs_horizon);
        f = smoothFunction(cost);
        to=tic;
        out_zerofpr2 = forbes(f, g, inputs, [], [], opt_zerofpr2);
        time_history(i)=toc(to)*1000;% get time in ms
        iteration_history(i)=out_zerofpr2.solver.iterations;
        inputs = out_zerofpr2.x;
        
        
        optimal_input=out_zerofpr2.x(1:trailer_controller.model.number_of_inputs);
        disp(['The optimal input is[' num2str(optimal_input(1)) ' ; ' num2str(optimal_input(2)) ']']);
        
        state = trailer_controller.model.get_next_state_double(state, optimal_input);
        state_history(:, i) = state;
    end
    
    disp("Final state:")
    disp(state)
    
    clear('sim'); % remove the simulator so it unloads the shared lib
end

